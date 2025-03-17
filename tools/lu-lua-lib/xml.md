
# `xml`

A simple library for parsing XML. Depends on the `classes` library.

## Classes

### `xml.element`

Represents a single XML element. Subclasses `xml.node`. Possesses the following properties (along with those inherited from `xml.node`):

* **`node_name`:** The tagname.
* **`attributes`:** A map of attribute names to attribute values.
* **`children`:** An array of child nodes.
* **`self_closed`:** Boolean value indicating whether this element was self-closing when parsed.

As the `classes` library currently lacks functionality for protected or private members, the above fields are public, but should only be modified via the appropriate member functions.

#### Member functions

##### `elem:append_child(node)`

Removes `node` from its parent node, and then appends it to this element.

##### `elem:children_by_node_name(name)`

Returns an array of all child elements whose node name is `name`.

##### `elem:for_each_attribute(functor)`

Invokes `functor` once for each attribute on the element, passing the attribute name and value as arguments. If `functor` returns `false` (not `nil`), iteration stops early.

##### `elem:for_each_child(functor)`

Invokes `functor` for each child node, passing the child node as an argument. If `functor` returns `false` (not `nil`), iteration stops early.

##### `elem:for_each_child_element(functor)`

Invokes `functor` for each child element, passing the child element as an argument. If `functor` returns `false` (not `nil`), iteration stops early.

##### `elem:remove()`

Removes `elem` from its parent.

##### `elem:remove_child(node)`

Removes `node` from this element's child list. Throws an error if `node` is not one of this element's children.

### `xml.node`

Base class for objects parsed from an XML document. Possesses the following properties:

* **`parent`:** The parent element, if any.

### `xml.parser`

An object which can be used to parse XML strings, retrieving the root node.

The constructor accepts an optional `options` argument: a table, which may contain the following properties:

* **`entities`:** A map of case-sensitive entity names to the text data that these entities represent. If this conflicts with the names of any built-in entities (e.g. `lt`), it will be shadowed by them.

#### Member functions

##### `parser:parse(str)`

Parses `str` as an XML string. After parsing, the following properties will be set on the parser:

* **`prologue`:** Either `nil`, or a table with the following properties:
  * **`version`:** The version number in the XML declaration, if there was one.
  * **`encoding`:** The text encoding in the XML declaration, if there was one.
  * **`standalone`:** A boolean value indicating whether `standalone` in the XML declaration was `yes` or `no`; or `nil` if `standalone` wasn't specified.
* **`root`:** The root element of the XML document.

Notes:

* Comments are not retained in the final parsed data.
* CDATA sections are parsed into `xml.text` objects, indistinguishable from ordinary character data post-parse.
* Doctypes are not supported.
* Identifiers within XML are held to the same requirements as in the XML 1.1 spec, but nothing is done to handle character encoding. Whatever Lua does when told to match `\u{200D}` is what you'll get.
* Processing instructions are not supported.
* Unrecognized XML entities trigger a parse error.

### `xml.text`

Represents a single piece of text data. Subclasses `xml.node`.

The `data` property is the content of the text data.

## Implementation notes

The parser is single-pass with no lexer.

The parser supports a "checkpoint" feature which returns "checkpoint" objects: these objects must be captured in to-be-closed variables (i.e. `local foo <close> = ...`) and are similar to scope guards and RAII objects in C++, automatically rewinding the parser when destroyed. To prevent the rewind, call `checkpoint:commit()` on the checkpoint instance.