
# `gcc_wrappers`

An attempt at creating a more type-safe interface to GCC's `tree` nodes, which are essentially just tagged `union`s. My current focus is on the GENERIC IR (not GIMPLE) and on dealing with C compilation (not C++).

Goals:

* Stricter static typing
* Improved ergonomics
* Improved version independence using compile-time version checks

This wrapper has some dependencies on my personal "helper" library (the `lu` folder).

## Defects

* I've only developed this plug-in for use when compiling C code with GCC. Several accessors don't support C++, and you should expect run-time link errors when trying to load it while compiling C++ code. See [FUTURE CPP SUPPORT.md](FUTURE%20CPP%20SUPPORT.md) for details.

## Usage example

Suppose we want to generate this C function:

```c
/*

   Assume:
   
   struct T {
      int foo[10];
   };
   
*/

void __generated_function_T(struct T* __arg_0) {
   for(int i = 0; i <= 9; ++i)
      (*__arg_0)[i] = i + 3;
}
```

GENERIC, the lower-level IR that GCC uses and that we'll be generating code in, doesn't have a way to natively represent `for` loops (outside of language-specific stuff that only gets used temporarily during parsing), so what we generate in GENERIC would look like this if translated back to C:

```c
void __generated_function_T(struct T* __arg_0) {
   int i = 0;
   goto test;
body:
   (*__arg_0)[i] = i + 3;
   ++i;
test:
   if (i <= 9)
      goto body;
   else
      goto exit;
exit:
}
```

The code to generate that function looks like this. (We actually end up generating a redundant anonymous block enclosing the `for` loop, which can be more convenient to work with when generating more complex functions.)

```c++
namespace gw {
   using namespace gcc_wrappers;
}

// Generate `void __generated_function_T(T* __arg_0)` for some type T:

gw::decl::function make_function(gw::type type) {
   const auto& ty = gw::builtin_types::get();
   auto int_type  = ty.basic_int;
   auto void_type = ty.basic_void;

   auto result = gw::decl::function(
      std::string("__generated_function_" + type.name()),
      gw::type::function(
         // return type:
         void_type,
         // argument types:
         type.add_pointer()
      )
   );
   auto dst = result.as_modifiable();
   dst.set_result_decl(gw::decl::result(void_type));
   
   gw::expr::local_block root_block;
   auto statements = root_block.statements();
   
   // Assume that the T type has a member `int foo[10]`.
   // Let's create a loop to set all of those integers 
   // to `i + 3`.
   //
   {
      //
      // First, we create the loop, specifying the value 
      // type of its counter variable. Then, we can set 
      // the counter bounds.
      //
      gw::flow::simple_for_loop loop(int_type);
      loop.counter_bounds = {
         .start     = 0,
         .last      = 9,
         .increment = 1,
      };
      
      //
      // At this point, the loop has created declarations 
      // for its counter variable and internal labels. You 
      // can use the counter variables in expressions, and 
      // you can generate `gw::expr::go_to_label`s if you 
      // need to `break` or `continue`.
      //
      
      gw::statement_list loop_body;
      
      loop_body.append(
         //
         // `__arg_0->foo[i] = i + 3`, expressed by way of 
         // the form `(*__arg_0).foo[i] = i + 3`:
         //
         gw::expr::assign(
            result.nth_argument(0).as_value().deference()
               .access_member("foo").access_array_element(loop.counter.as_value()),
            //
            // =
            //
            loop.counter.as_value().add(
               gw::constant::integer(int_type, 3)
            )
         )
      );
      
      //
      // We want to tell the loop to generate its local 
      // block and relevant expressions, including those 
      // expressions to actually place its labels somewhere.
      //
      loop.bake();
      
      statements.append(loop.enclosing);
   }
   
   //
   // We've generated the code we want to, so let's now go 
   // ahead and finalize our function.
   //
   // We want this function to make it into the object file, 
   // so let's mark it as non-extern before we commit its 
   // root block.
   //
   dst.set_is_defined_elsewhere(false);
   dst.set_root_block(root_block);
   
   //
   // As a bonus, let's also bind the function to the current 
   // scope, so that name lookups (`lookup_name`) can find it.
   //
   // The "current scope" is, AFAIK, whatever scope GCC is 
   // parsing at the time this code runs. I don't know of a more 
   // direct or controlled way to bind declarations to, say, the 
   // file scope specifically.
   //
   dst.introduce_to_current_scope();

   return result;
}
```

The equivalent using raw `tree` nodes, in GCC 11.4.0, would look roughly like this. (This is me working backwards from the properly functioning code in my wrappers, translating without testing, so don't expect to be able to copy and paste this into your IDE and have it work perfectly. It's presented for illustrative purposes only.)

```c++
tree make_function(tree type) {
   auto fn_type = build_function_type_list(
      void_type_node,
      build_pointer_type(type),
      NULL_TREE
   );
   tree fn_name = NULL_TREE;
   {
      std::string name = "__generated_function_";
      
      auto node = TYPE_NAME(type);
      if (node != NULL_TREE) {
         if (TREE_CODE(node) == TYPE_DECL)
            node = DECL_NAME(node);
         if (node != NULL_TREE) {
            if (TREE_CODE(node) == IDENTIFIER_NODE)
               name += IDENTIFIER_POINTER(node);
         }
      }
      
      fn_name = get_identifier(name.c_str());
   }
   
   auto fn_decl = build_fn_decl(fn_name, fn_type);
   {
      // Gotta build the argument PARM_DECLs manually.
      // My wrappers automate this based on the function type, but we'll 
      // do it entirely by hand here for clarity.
      
      auto arg_type = build_pointer_type(type);
      auto arg_decl = build_decl(
         UNKNOWN_LOCATION,
         PARM_DECL,
         get_identifier("__arg_0"),
         arg_type
      );
      DECL_ARG_TYPE(arg_decl) = arg_type;
      DECL_CONTEXT(arg_decl)  = fn_decl;
      
      DECL_ARGUMENTS(fn_decl) = arg_decl; // NOTE: this is a chain
   }
   
   // Make return value.
   {
      tree res_decl = build_decl(
         UNKNOWN_LOCATION,
         RESULT_DECL,
         NULL_TREE, // unnamed
         void_type_node
      );
      DECL_RESULT(fn_decl)   = res_decl;
      DECL_CONTEXT(res_decl) = fn_decl;
   }
   
   auto statements = alloc_stmt_list();
   auto root_block = build3(
      BIND_EXPR,
      void_type_node, // return type
      NULL,           // vars (accessible via BIND_EXPR_VARS(expr))
      statements,     // STATEMENT_LIST node
      NULL_TREE       // BLOCK node
   );
   
   tree statement_it = tsi_start(statements);
   tree loop_counter = NULL_TREE;
   
   {
      // Create the VAR_DECL, and then create a DECL_EXPR to actually place it 
      // inside the function.
      loop_counter = build_decl(
         UNKNOWN_LOCATION,
         VAR_DECL,
         get_identifier("__i"),
         integer_type_node
      );
      tsi_link_after(
         &statement_it,
         build_stmt(UNKNOWN_LOCATION, DECL_EXPR, loop_counter),
         TSI_CONTINUE_LINKING
      );
      
      // Set the initial value for the loop counter.
      DECL_INITIAL(loop_counter) = build_int_cst(integer_type_node, 0);
      
      // Make our LABEL_DECLs.
      auto label_body = build_decl(
         UNKNOWN_LOCATION,
         LABEL_DECL,
         NULL_TREE,
         void_type_node
      );
      auto label_test = build_decl(
         UNKNOWN_LOCATION,
         LABEL_DECL,
         NULL_TREE,
         void_type_node
      );
      auto label_exit = build_decl(
         UNKNOWN_LOCATION,
         LABEL_DECL,
         NULL_TREE,
         void_type_node
      );
      DECL_CONTEXT(label_body) = fn_decl;
      DECL_CONTEXT(label_test) = fn_decl;
      DECL_CONTEXT(label_exit) = fn_decl;
      
      tsi_link_after(
         &statement_it,
         build1(GOTO_EXPR, void_type_node, label_test),
         TSI_CONTINUE_LINKING
      );
      
      // Append loop body: (*__arg_0).foo[__i] = __i + 3;
      tsi_link_after(
         &statement_it,
         build_modify_expr(
            UNKNOWN_LOCATION, // expression source location
            
            // LHS:
            build_array_ref(
               UNKNOWN_LOCATION,
               build_component_ref(
                  UNKNOWN_LOCATION,
                  build1(
                     INDIRECT_REF,
                     type,
                     DECL_ARGUMENTS(fn_decl)
                  ),
                  get_identifier("foo"),
                  UNKNOWN_LOCATION
                  #if GCCPLUGIN_VERSION_MAJOR >= 13 && GCCPLUGIN_VERSION_MINOR >= 1
                     ,
                     UNKNOWN_LOCATION
                     #if GCCPLUGIN_VERSION_MAJOR >= 14 && GCCPLUGIN_VERSION_MINOR >= 3 // future
                        ,
                        true
                     #endif
                  #endif
               ),
               loop_counter
            ),
            
            NULL_TREE,        // original type of LHS (needed if it's an enum bitfield)
            NOP_EXPR,         // operation (e.g. PLUS_EXPR if we want to make a +=)
            UNKNOWN_LOCATION, // RHS source location
            
            // RHS:
            build2(
               PLUS_EXPR,
               integer_type_node,
               loop_counter,
               build_int_cst(integer_type_node, 3)
            ),
            
            NULL_TREE         // original type of RHS (needed if it's an enum)
         ),
         TSI_CONTINUE_LINKING
      );
      
      // Increment loop counter.
      tsi_link_after(
         &statement_it,
         build2(
            PREINCREMENT_EXPR,
            integer_type_node,
            loop_counter,
            build_int_cst(integer_type_node, 1)
         ),
         TSI_CONTINUE_LINKING
      );
      
      tsi_link_after(
         &statement_it,
         build1(LABEL_EXPR, void_type_node, label_test),
         TSI_CONTINUE_LINKING
      );
      
      // When GCC 11.4.0 compiles `for` loops, it actually will 
      // generate a ternary operator (`COND_EXPR`) whose branches 
      // are `goto` instructions to continue or break the loop. 
      // We'll do the same.
      tsi_link_after(
         &statement_it,
         build3(
            COND_EXPR,
            void_type_node,
            build2(
               LE_EXPR,
               integer_type_node,
               loop_counter,
               build_int_cst(integer_type_node, 9)
            ),
            build1(GOTO_EXPR, void_type_node, label_body),
            build1(GOTO_EXPR, void_type_node, label_exit)
         ),
         TSI_CONTINUE_LINKING
      );
      
      tsi_link_after(
         &statement_it,
         build1(LABEL_EXPR, void_type_node, label_exit),
         TSI_CONTINUE_LINKING
      );
   }
   
   //
   // We've generated the code we want to, so let's now go 
   // ahead and finalize our function.
   //
   // We want this function to make it into the object file, 
   // so let's mark it as non-extern before we commit its 
   // root block.
   //
   DECL_EXTERNAL(fn_decl) = 0;
   
   //
   // Now we have to commit its root block... but that means 
   // setting up scope information. This would end up being 
   // very complicated if we had more than one block in this 
   // function. We have to create a tree of BLOCK nodes that 
   // exactly reflects the tree of BIND_EXPR nodes, including 
   // BIND_EXPRs that may be operands of ternaries or other 
   // expressions. The hierarchy is as follows:
   //
   //  - BIND_EXPR_BLOCK links a BIND_EXPR to its BLOCK.
   //
   //  - BLOCK_SUPERCONTEXT links a BLOCK to its parent BLOCK 
   //    or FUNCTION_DECL.
   //
   //  - BLOCK_SUBBLOCKS links a parent BLOCK to its first 
   //    child BLOCK.
   //
   //  - DECL_INITIAL links a FUNCTION_DECL to its first child 
   //    BLOCK.
   //
   //  - BLOCK_CHAIN links a block to its next sibling.
   //
   //  - BLOCK_VARS is a tree-chain of all variables local to 
   //    a block (based on the location of DECLARE_EXPRs).
   //
   //  - DECL_CONTEXT links a VAR_DECL to its parent BLOCK or 
   //    FUNCTION_DECL. It also links a LABEL_DECL to its 
   //    containing FUNCTION_DECL (irrespective of the block 
   //    hierarchy).
   //
   // We set DECL_CONTEXT on the labels and function args when 
   // we created them, so we'll just focus on everything else.
   //
   tree root_scope = make_node(BLOCK);
   BIND_EXPR_BLOCK(root_block) = root_scope;
   BLOCK_SUPERCONTEXT(root_scope) = fn_decl;
   DECL_INITIAL(fn_decl) = root_scope;
   BLOCK_VARS(root_scope) = loop_counter; // NOTE: this is a chain
   DECL_CONTEXT(loop_counter) = root_scope;
   //
   // Oh, and if we get literally anything about this process 
   // wrong, GCC will crash once it starts actually trying to 
   // process the IR, whether for optimizations or to finally 
   // produce bytecode. Typically the crash will be an assert 
   // call, but you won't have much debug information to work 
   // with, and many of GCC's assertions test five conditions 
   // at once rather than asserting each one separately. That 
   // can make mistakes with function generation very hard to 
   // investigate and fix.
   //
   
   //
   // We need to update TREE_SIDE_EFFECTS on each BIND_EXPR 
   // as well.
   //
   walk_tree(
      &root_block,
      [](tree* current_ptr, int* walk_subtrees, void* data) -> tree {
         tree root_block = *(tree*)data;
         if (*current_ptr == root_block)
            return NULL_TREE;
         
         if (TREE_SIDE_EFFECTS(*current_ptr)) {
            TREE_SIDE_EFFECTS(root_block) = 1;
            return root_block; // stops iteration
         }
         return NULL_TREE;
      },
      &root_block,
      nullptr
   );
   
   //
   // My wrappers automate all of the above setup for scoping 
   // and side-effect flags, walking the contents of the root 
   // block when you attach it to the function in order to 
   // find and handle all nested blocks and declarations that 
   // need handling.
   //
   
   if (!DECL_STRUCT_FUNCTION(this->_node)) {
      //
      // We need to allocate a "struct function" (i.e. a struct that describes 
      // some stuff about the function) in order for the function to be truly 
      // complete. (Unclear on what purpose this serves exactly, but I do know 
      // that the GCC functions to debug-print a whole FUNCTION_DECL won't even 
      // attempt to print the PARM_DECLs unless a struct function is present.) 
      //
      // Allocating a struct function triggers "layout" of the FUNCTION_DECL 
      // and its associated DECLs, including the RESULT_DECL. As such, if there 
      // is no RESULT_DECL, then GCC crashes.
      //
      assert(DECL_RESULT(fn_decl) != NULL_TREE);
      allocate_struct_function(fn_decl, false);
   }
   
   // NOTE: If we were generating a nested function, we'd instead want to call 
   // `cgraph_node::get_create`, I believe.
   cgraph_node::finalize_function(this->_node, false);
   
   //
   // Whew. We've finally finalized the root block, so let's introduce this new 
   // function to the current scope, for name lookups.
   //
   // `c_bind` is meant for declarations, not definitions, AFAICT anyway. It 
   // therefore does some things that we maybe don't want, like changing the 
   // function's "extern" status.
   //
   auto was_extern = DECL_EXTERNAL(this->_node);
   c_bind(DECL_SOURCE_LOCATION(this->_node), this->_node, true); // c/c-tree.h
   DECL_EXTERNAL(this->_node) = was_extern;
   rest_of_decl_compilation(this->_node, true, false); // toplev.h

   //
   // And *now* we're done.
   //

   return fn_decl;
}
```

It's nice not to have to deal with *all of that* every time you need to generate code.

Anyway, onto more information about the library.

## General design

There are three basic categories for these wrappers.

* **General wrappers** will wrap a single `tree`, which is guaranteed not to be `NULL_TREE` unless the wrapper has been left in a moved-from state. The base class for all such wrappers is `node`, with subclasses for specific wrapper types (e.g. `attribute`, `decl::variable`, `type::floating_point`).
  
  Aside from copying or rebinding to another wrapper, a new general wrapper can only be instantiated through three means:
  * Dereferencing an optional wrapper.
  * Calling <code><var>wrapper_type</var>::wrap</code> passing a raw `tree`. We will `assert` that the tree is non-null and of the correct type for the chosen wrapper.
  * Using non-copy constructors, when available, to create new `tree`s of the wrapped type.
* **Optional wrappers** will wrap a single `tree`, which may be `NULL_TREE`. These wrappers dereference to reference-like wrappers with an interface similar to `std::optional`. We define an optional wrapper for each reference-like wrapper following the naming convention <code>optional_<var>basename</var></code> (e.g. `node` and `optional_node`; `decl::field` and `decl::optional_field`; `type::pointer` and `type::optional_pointer`).
  
  Optional wrappers can be constructed from one another, from a general wrapper, or from a bare `tree`.
* **View-like wrappers** are a special case offered for attribute lists (`attribute_list`) and chains (`chain`).
  * The `chain` wrapper is a view because tree nodes of (more or less) any type can also be chains or links therein; "chain-ness" is orthogonal to the node type and doesn't fit within the `node` wrapper type hierarchy.
  * The `attribute_list` wrapper is a view because access to attributes is more ergonomic this way. I reserve the right to wrap the attribute-owning `DECL` or `TYPE` directly in the future; but for now, `attribute_list` wraps the list head, and it does so in a special-case manner because having to distinguish between `attribute_list` and `optional_attribute_list` would've been clumsy given the interface that we offer for attribute lists.

## Class hierarchy

Most of the namespaces are fairly simple: `constant` holds classes that wrap `*_CST` nodes; `decl` holds classes that wrap `*_DECL` nodes; `expr` holds classes that wrap `*_EXPR` nodes; and `type` holds classes that wrap `*_TYPE` nodes.

The `value` wrapper is something of a special-case. All `EXPR`s and `CST`s can be used as values (e.g. operands to other expressions) and have a value type, but only some `DECL`s can be used the same way. As such, all `constant` and `expr` wrappers subclass `value`, but the `decl::base_value` wrapper does not: it subclasses `decl::base` and offers an `as_value()` accessor for converting to `value`.

In general, `expr` wrappers exist for "advanced" operations, like function calls, declaration placement[^decl-exprs], `goto`s, and ternaries; while `value` is used for "simple" operations, like unary and binary operators, member accesses, and array accesses.

[^decl-exprs]: `DECL` nodes indicate the existence, source code location, and content of a declaration, but they don't have a "location" in *practical* terms. `DECLARE_EXPR` and `LABEL_EXPR` are needed to actually place a `DECL` in a particular scope and at a particular position relative to other expressions in that scope.

### General wrappers

* `node`
  * `attribute`
  * `identifier`
  * `list_node`
  * `scope`
  * `statement_list`
  * `value`
    * `constant::base`
      * `constant::floating_point`
      * `constant::integer`
      * `constant::string`
    * `expr::base`
      * `expr::assign`
      * `expr::call`
      * `expr::declare`
      * `expr::declare_label`
      * `expr::go_to_label`
      * `expr::local_block`
      * `expr::return_result`
      * `expr::ternary`
  * `decl::base`
    * `decl::base_value`
      * `decl::field`
      * `decl::param`
      * `decl::result`
      * `decl::variable`
    * `decl::function`
    * `decl::label`
    * `decl::type_def`
  * `type::base`
    * `type::array`
    * `type::container`
      * `type::record`
      * `type::untagged_union`
    * `type::fixed_point`
    * `type::floating_point`
    * `type::function`
      * `type::method`
    * `type::integral`
      * `type::enumeration`
      * `type::integer`
    * `type::pointer`

### Other types

* `::lu::singleton`
  * `builtin_types`
  * `events::on_type_finished`
* `environment::c::dialect`
* `environment::c_family::language`
* `flow::simple_for_loop`
* `flow::simple_if_else_set`
* `attribute_list`
* `chain`

## Lifetime

These wrappers function akin to smart pointers, wrapping `tree` nodes created on GCC's heap. Memory management is subject to GCC's own design. My research on that is incomplete, and the wrappers offer no facilities for dealing with it directly.

GCC uses garbage collection internally ("GGC"). The allocator keeps track of every tree node and periodically, GCC will start at all top-level objects and traverse over them to check which tree nodes are reachable. Unreachable nodes are deleted. Garbage collection doesn't run spontaneously, but rather must be triggered explicitly via calls to `ggc_collect`. This means that you don't need to explicitly delete tree nodes that you create. Some operations may create and discard nodes (e.g. starting at a type node for `T` and getting a type node for `const T*` via `type.add_const().add_pointer()`); this is fine, because the intermediate nodes can be garbage-collected later.

[The `_GTY()` macro](https://gcc.gnu.org/onlinedocs/gccint/Type-Information.html) is used to annotate top-level objects (the [roots](https://gcc.gnu.org/onlinedocs/gccint/GGC-Roots.html)). The macro acts as a signal to `gengtype`, a GCC code generation tool that understands a limited subset of C++.

If you have singletons or globals that may hold on to GGC-controlled objects, then you'll need to mark them with `_GTY()` and [run `gengtype` in plug-in mode](https://gcc.gnu.org/onlinedocs/gccint/Files.html) to generate garbage-collection code.[^plugin-gengtype-when] Because `gengtype` only supports a very limited subset of C++ syntax, you may need to [hand-write your own code](https://gcc.gnu.org/onlinedocs/gccint/User-GC.html) to help the garbage collector traverse your objects.

[^plugin-gengtype-when]: GCC seems to have had multiple plug-in APIs over the years. The mention of `gengtype` having a "plugin mode" [dates back to 2009](https://github.com/gcc-mirror/gcc/commit/bd117bb6b44870ca006eb12630a454302873674e) (circa GCC 4). The current plug-in API dates back to around 2009 as well, so safe to say that the `gengtype` integration described would apply to the current plug-in API.

### Garbage-collected GCC objects

If your singleton stores these objects, then it'll need to pass them to GGC functions in handwritten overloads for `gt_ggc_mx` and friends.

This list is non-exhaustive.

* `tree`
  * NOTE: Given a `location_t loc`, `LOCATION_BLOCK(loc)` is a `tree`. It seems like GCC prefers to capture it in a local and pass the local to `gt_ggc_mx` and friends, rather than passing the macro expression directly.
* `gimple_seq`
* `rtx`


### An overview of GCC's memory management

As of this writing, GGC and `gengtype` are, uh, *not terribly well-explained*, so here's my try at actually communicating how they work.

GGC is a mark-and-sweep garbage collector. It starts at "root" objects and traverses down through them to mark all seen garbage-collectible objects, using `gengtype`-generated code to navigate the structures. Any objects that aren't marked by the end of a pass are assumed to be unreachable and are disposed of.

Objects are garbage-collectible if they're managed using any of these functions:

* `ggc_alloc`
* `ggc_vec_alloc`
* `ggc_cleared_alloc`
* `ggc_cleared_vec_alloc`
* `ggc_alloc_atomic`
* `ggc_alloc_no_dtor`
* `ggc_alloc_string`
* `ggc_strdup`
* `ggc_realloc`
* `ggc_delete`
* `ggc_free`

While reading about `gengtype`, you'll see references to "PCH," which stands for "pre-compiled header." The particular way PCHs are talked about in the documentation won't make sense unless you know that [GCC implements pre-compiled headers as snapshots of the entire heap](https://stackoverflow.com/a/12438040)[^pch-savestate], akin to savestates in a game console emulator or snapshots in a VM. Whenever you see "a PCH" in documentation related to GGC or `gengtype`, mentally substitute the phrase out for "a savestate."

[^pch-savestate]: PCH savestates are created with `gt_pch_save` and loaded with `gt_pch_restore`.

If you have a data structure that needs to act as a GC root, e.g. a singleton that refers (directly or indirectly) to garbage-collectible objects, then you'll need to annotate it for `gengtype`. If your singleton is too complex for `gengtype` to understand, then you can annotate it with the `user` option, to tell `gengtype` that you'll be supplying user-defined functions to mark the garbage-collectible objects that your singleton refers to.

```c++
extern GTY((user)) my_struct_type my_struct_instance;
```

The three functions you'll need to define are:

* `void gt_ggc_mx(my_struct* instance);`
  * This function marks the garbage-collectible objects that `instance` refers to. All you have to do is recursively call it (i.e. `gt_ggc_mx(instance->field_to_mark)`) on any fields that are pointers to collectible objects.
* `void gt_pch_nx(my_struct* instance);`
  * This function is basically the same thing as `gt_ggc_mx`, but for operations related to a PCH (i.e. a savestate). Recursively call `gt_pch_nx` on the pointers to collectible objects.
* `void gt_pch_nx(my_struct* instance, gt_pointer_operator op, void* cookie);`
  * This function is used for operations related to a PCH (i.e. a savestate). It may passively read your struct instance (e.g. to write a new savestate to disk), or it may actively modify your struct instance (e.g. to load a savestate into memory, performing pointer fix-ups as necessary).
  * This function needs to call the given pointer operator on every garbage-collectible field. That is: given some garbage-collectible type `T`, if your object contains some `T* field`, you need to call `op(&field, nullptr, cookie)` such that the operator receives a `T**`.

Moving from prose to code, the functions look like this:

```c++
void gt_ggc_mx(my_struct* instance) {
  gt_ggc_mx(instance->field_to_mark);
}

void gt_pch_nx(my_struct* instance) {
  gt_pch_nx(instance->field_to_mark);
}

void gt_pch_nx(my_struct* instance, gt_pointer_operator op, void* cookie) {
  op(&(instance->field_to_mark), NULL, cookie);
}
```

You'll note that you can only mark garbage-collectible fields, or forward an operation to run on garbage-collectible fields; there doesn't appear to be any way to store or restore your object's state here. This can be seen on GCC internals as well. For example, their `vec` template is `GTY((user))` since it's a template, and the only thing its GGC functions do is recurse on the vector elements; nothing is done to store the vector's length. It's obvious, then, how GGC marks collectible objects, and how it does pointer fix-ups; but it's not clear how it actually stores object data. Perhaps savestates can only record the contents of collectible objects, such that a singleton would be wiped clean unless it's under GGC's control? (But `vec` doesn't *seem* to be allocated through `ggc_alloc` and friends? And don't some `tree` structures store `vec`s?)

There also exists a `ggc_register_root_tab` function that is noted as being "useful for some plugins." No clue when or how to use it.

## Potential future plans

* More `flow` wrappers to simplify common control flow structures
  * `while`
  * `do` ... `while`
  * `switch`/`case`, including with fall-through
    * We need not support things like Duff's device here.
* Consider renaming `decl::base::is_defined_elsewhere` and friends to `decl::base::is_extern`. Some GCC comments seem to suggest that `DECL_EXTERN` doesn't correspond 1:1 with the C/C++ `extern` keyword, else I'd have used that name to start with.

### Missing features

* `ARRAY_RANGE_REF`. There's a member function declaration on `value` but no implementation.