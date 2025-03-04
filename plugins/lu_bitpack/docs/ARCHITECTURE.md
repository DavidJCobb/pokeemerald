
# Plug-in architecture

This plug-in is designed to generate code to bitpack (read or save) identifiers that exist at file scope. To that end, it needs to accept globally set options as well as options for each to-be-serialized value. The latter options can be defined directly on values, or defined on their types, and so are called "data options" (as opposed to "value options," etc.).

Code generation is assumed to be per translation unit: you can't generate multiple sets of functions in a single translation unit, for example.

## Options

### Global options

Global options are set via the custom `set_options` pragma. They are stored in the `basic_global_state` singleton.

### Data options

Data options fall into two basic categories: those options that are usable on basically any entity; and *typed* data options, which are options that imply a type (e.g. integral options).

Data options take three forms as the plug-in executes:

* **Attributes:** Once bitpacking has been enabled (via the custom `enable` pragma), our attribute handlers activate. To the fullest extent possible, attribute handlers will not allow an entity to retain an attribute if the attribute is used incorrectly; we write sentinel attributes onto the entity in order to simplify error checking and error handling later on. This allows us to report problems as attributes are parsed, and to report problems with entities that have been given bitpacking attributes but haven't yet been added to the list of values to serialize.

  (When performing validation, attribute handlers only check details that are visible from the handler. For example, an individual attribute handler doesn't check for the presence of other, mutually conflicting, attributes on the same entity; and if the annotated entity is a declaration, we may check basic constraints on the type (e.g. no integral attributes on non-integral types), but we don't check for preexisting attributes on the type. More detailed validation is performed when we compute a declaration's data options, per the below.)
  
  Attribute handlers may transform the attribute arguments from a form that is valid for C++ into a form that is more directly usable by the plug-in internals. For example, the `lu_bitpack_transform` attribute takes a single argument: a string literal, with options specified as key/value pairs. The attribute handler transforms this into two arguments: the raw identifiers of the transform functions.

* **Requested typed data options:** These data structures encode the options that were requested by the user, as extracted from the attributes. For example, an integral value will have `std::optional`s for the minimum and maximum values and for an explicitly specified bitcount.

* **Computed data options:** These are the final data options that apply to a given (variable or field) declaration. Some options may be inferred from other options and/or from the declaration's type. For example, if you don't explicitly specify a bitcount for an integral value, we will compute the bitcount to use based on the minimum and maximum values you specified (if any), the value's type, and, if the value is a bitfield, its width in bits. Similarly, if a to-be-serialized value of integral type has no bitpacking options whatsoever, we would auto-generate integral data options for it.
  
  Computing these options is potentially a multi-stage process. A declaration first pulls any options specified on its type, and then overrides those with options specified on the declaration itself. If the declaration's type is a `typedef`, then attributes applied to the `typedef` override attributes applied to the original type; and this applies recursively, for `typedef`s of `typedef`s.
  
  Additionally, data options, once computed, may be considered invalid, e.g. if the declaration (or its type, or any transitive `typedef`s) had any invalid attributes (as detected by the attribute handlers), or if their bitpacking options are individually valid but mutually conflicting, or if their bitpacking options are invalid in ways that can't be detected from attribute handlers (per the below).

Notably, there are a few cases where attribute handlers can't perform full validation, and where by the time full validation is possible, applying sentinel attributes to mark an entity as invalid is no longer possible. Chiefly these cases involve the bitpacking options for tagged unions:

* An externally-tagged union needs to specify a valid tag: a member of the enclosing struct, which appears before the union. However, `DECL_CONTEXT` isn't ready yet when attribute handlers run: we don't know if the union is inside of a struct, let alone what already-parsed members that struct has. We have to validate this requirement when the struct is fully parsed (`PLUGIN_FINISH_TYPE`)

* An internally-tagged union needs to specify a valid tag: the union's members must all be structs, must have a non-zero number of their first members in common, and the tag must be one of those. If `lu_bitpack_union_internal_tag` is applied to a union type declaration, then the union won't be fully parsed at the time the attribute handler runs; again, we must use `PLUGIN_FINISH_TYPE` to validate the union. However, if the attribute is applied to a field or variable declaration of union type, and if the union type is complete, then the attribute handler can perform full validation; these conditions are generally all met by `union { ... } foo`.
  
  A slight complication here is that if it's the union *type* that's marked as internally tagged, we can't apply sentinel attributes to it to mark it as invalid. By the time `PLUGIN_FINISH_TYPE` runs, the type's attributes are no longer alterable.

* A tagged union's members must be either omitted from serialization, or given a unique ID (the tag value). This runs into the same issues as validating internally-tagged unions' specified tags. We also want to disallow the "unique ID" attribute on members of a non-union, which runs into similar problems as with validating externally-tagged unions' specified tags.

* A union cannot be both internally and externally tagged. GCC has a built-in function to make attributes mutually exclusive, but in that case, the attribute handlers fail to run at all. Additionally, it doesn't cover the case of a union type being marked as internally tagged, and then a struct field of that type being marked as externally tagged. Ergo we have to validate this from a `PLUGIN_FINISH_TYPE` handler.

* A to-be-transformed value must be addressable, i.e. it cannot be a bitfield. We may be able to validate that from the attribute handler for transform options, but we'd end up overlooking the case of a bitfield that does not itself specify transform options, but rather is influenced by them because they were applied to the bitfield's type.

----

Data options can be computed at any time, but during codegen, they are stored on `decl_descriptor` objects, which wrap all to-be-serialized `DECL`s (variables and fields).

## Code generation

### Output

Code generation produces the following results:

* **Per-sector functions:** Our plug-in supports generating code to deal with bitstreams that are split into multiple fixed-size pieces, or "sectors." If you use this functionality, then each sector gets its own "read" and "save" function. If you don't use this functionality, then we think in terms of your entire bitstream being one sector with no maximum size.

* **Whole-struct functions:** If an entire (non-anonymous) struct can fit whole in a sector, then we generate functions ("read" and "save") that can be called to serialize the struct member-by-member. This is done to reduce the size of the generated code when dealing with structs that are used more than once within the serialized output. These functions are remembered per-type and reused.
  
  Whole-struct functions, then, will only ever be called by per-sector functions or by other whole-struct functions.

* **Top-level functions:** These are the functions that the user directly asks us to generate, taking a pointer to a buffer and a sector index as arguments. These initialize a bitstream state struct to operate on the buffer, and then use a generated if/else tree to check the sector index and call the appropriate per-sector function, passing a pointer to the bitstream state struct.
  
  The intended design of these functions is such that as long as you pass the right buffer for each sector (i.e. `__generated_read(sector_0_buffer, 0)`), it doesn't matter what chronological order you read or save the sectors in.


### Passes

Code generation is done in multiple passes.

#### Serialization items

A <dfn>serialization item</dfn> is a data structure similar to a path, representing either zero-padding, or a to-be-serialized value (which may be an aggregate). In the latter case, each path segment represents a single declaration (i.e. a variable or field &mdash; specifically, the `decl_descriptor` wrapping it), and can optionally be paired with array access info and a condition. Serialization items are stored in flat lists, and when such an item represents an aggregate, it can be <dfn>expanded</dfn> into multiple serialization items, each representing the elements of that aggregate.

Array access info represents accesses to a single array element or to an array slice (i.e. a to-be-generated `for` loop). A segment can have multiple such info structures to represent access into nested arrays. For example, `foo[2][6][0:3]` is a single path segment.

Conditions are used when dealing with unions; they indicate a union tag to check, and the integer to equality-compare it to. (Similarly, padding items are generated for unions, to normalize the union's serialized size by padding smaller members out to match larger members; and so padding can also have conditions.)

Serialization items exist primarily to facilitate a specific feature of the plug-in: splitting a bitstream across multiple fixed-size sectors, with serialized structs or arrays potentially straddling a sector boundary. The sector splitting process is easiest to implement when dealing with serialization items: start with a list of serialization items representing the top-level variables to serialize, and if an item doesn't fit in the current sector, then either replace it with its own expansion and try again with the first of the expanded items (if the item *can* be expanded), or push the item to the next sector. If this feature wasn't one of the plug-in's goals, we could likely just generate code directly from the to-be-serialized values.

##### Edge-case: to-be-transformed values

Some values may be transformed as a result of their bitpacking options. When this occurs, serialization items represent members of the transformed value, not the original value. For example, if `foo` is a to-be-transformed value, then a serialization item that *appears* to represent `foo.bar` *actually* represents `__transformed_foo.bar`; the original `foo` may not even have a `bar` member.

##### Edge-case: unions

Tagged unions must be forcibly expanded, so that we generate serialization items with conditions in them (which in turn is important for dealing with re-chunked items and similar).

We could theoretically generate "whole-union" functions, similar to whole-struct functions. "Whole-union" functions for internally-tagged unions could be invoked as normal, while "whole-union" functions for externally-tagged unions would have to take the tag as an additional argument. This would probably be an improvement to code generation when dealing with named union types that appear multiple times in the serialized output, but it's not implemented at this time.

#### Re-chunked items

A <dfn>re-chunked item</dfn> is similar to a serialization item, except that each item represents *one* value to serialize, and the path is divided up slightly differently. Each "chunk" corresponds fairly directly with some to-be-generated code:

* **Array slice** chunks encode access into a single array rank, and represent a to-be-generated `for` loop. These only represent the loop itself, not anything inside it, and so these can never be the last chunk of an item.

* **Condition** chunks encode the same conditions as seen in serialization items; these conditions are just entire path segments now, rather than annotations to path segments.

* **Padding** chunks encode zero-padding.

* **Qualified decl** chunks encode member access: consecutive member accesses, with no array element accesses or conditions, are chunked together. For example, the value `foo.bar.baz` would comprise three segments in a serialization item, but only one qualified decl chunk in a re-chunked item.

* **Transform** chunks encode transformation of a value to another type. Serialization items do not encode transform options, but they can be looked up via a segment's `decl_descriptor`. These chunks represent an anonymous block wherein the transformed value is constructed. They do not represent serialization of the transformed value itself.

In order to generate code, we have to go from a flat list of serialization items to a tree of code blocks &mdash; loops, if/else trees, and so on. Re-chunked items are an intermediate step, being stored in a flat list but being divided by the same criteria by which we'd set up parent/child relationships.

#### Instruction nodes

<dfn>Instruction nodes</dfn> map fairly directly to the final code that we generate. A tree of instruction nodes can be used to generate a function body &mdash; a tree of code blocks, where the leaves are individual instructions for serializing individual values.

Instruction nodes often use `value_path` structures to refer to pertinent values (e.g. the array that an array-slice node loops over; the value that a transform node transforms; the value that a "single" node serializes), and `value_path`s are also used when converting from re-chunked items to nodes.

Some instruction nodes will pre-create variables. For example, an array-slice instruction node creates its loop counter in the node constructor, and a transform instruction node creates the variable for the transformed value in the node constructor. These variables are pre-created because `value_path`s use `decl_descriptor` pointers to refer to variables and values, and so we need to be able to refer to these variables in advance of actually generating code. (As an example of the contrary, a transform node may transform *through* one type to another, e.g. `A` transforms to `B` which transforms to `C`. The `A`-type value already exists &mdash; it's the thing we want to transform &mdash; and we pre-create a variable for the `C`-type value, because we may have nodes which refer (via `value_path`s) to it or to its members; but we don't need to pre-create a variable for the `B`-type value.)

The instruction node types are:

* **Array slice** nodes encode a `for` loop over an entire array or a slice thereof. These nodes only represent the loop itself; child and descendant nodes will generate the code to actually serialize array elements.

* **Padding** nodes represent zero-padding used for union branches.

* **Single** nodes store a single `value_path` to a to-be-serialized value, and generate the code to serialize just that one value.

* **Transform** nodes store a `value_path` to a to-be-transformed value, and represent the process of transforming that value for reading or serialization, but *not* the act of serialization itself. These nodes generate one or more anonymous blocks and local variables for transforming the value, and then child and descendant nodes actually serialize the value (in whole or in part).
  
  Accordingly, though the last chunk of a re-chunked item can be a transform chunk, when we convert the item to a node, we will implicitly generate a single node inside of the transform node, if the transform node would otherwise be a leaf.

* **Union-switch** nodes encode an if/else tree representing all of the branches used to serialize a tagged union. They store the `value_path` to the union tag, and a map of tag values to **union-case** nodes. The union-case nodes contain serialization instructions and act as the bodies of the to-be-generated `if` statements. (We generate if/else trees but choose the names "switch" and "case" for these nodes because... that's what they are. It's a set of branches wherein the same operand is repeatedly equality-compared to different values. If we actually generated a C-specific `switch` statement, it'd likely just compile down to an if/else tree anyway.)

### Nuances of sector splitting

The following data types currently can't be split across sectors:

* **Opaque buffers:** Would require making it possible for serialization items, re-chunked items, and single instruction nodes to encode the slicing of a value (as opposed to the slicing of an array). We'd then have to make it so that buffers are considered sliceable.

* **Strings:** Requires the same affordances as opaque buffers, with additional handling for strings. Bitstream functions for strings may want to normalize them, e.g. by guaranteeing a null terminator, or by nulling out the contents of an optionally-terminated string before reading it. These "fixup" operations would need to be separated out into their own bitstream functions and provided via global options. (We can't hardcode these behaviors into code generation, because some obscure proprietary character sets don't actually use 0x00 as their string terminator.)

* **Primitive values:** Simple integers and pointers can't be split across sectors. This much is obvious, but it's also relevant to what we're about to talk about.

If a value can't be expanded or split across sectors, then the entire value must be pushed to the next sector &mdash; potentially leaving unused bits at the end of the sector the value couldn't fit into. This must be taken into account, when measuring the size of the serialized data and comparing it to a naive `memcpy`.