
# `json`

A Lua library for parsing and serializing JSON. Depends on the `classes` library.

## `json.from(str)`

Parses `str` as JSON, returning the parsed object.

## `json.to(value, options)`

Converts `value` to JSON, returning the result as a string. Fails on an empty or whitespace-only string.

The `options` argument is optional and is a table of options. The `indent` option is an integer indicating the number of spaces by which to indent the value, though it's only applied if the `pretty_print` option is truthy.

An error is raised upon trying to stringify any Lua functions, or any `userdata` which lack a metatable with a `__tostring` metamethod.

Tables will be serialized as array literals if the following criteria are met, or as object literals otherwise.

* The table has a metatable with a `__json_type` property, whose value is the string `"array"`.
* The table's pairs are all integers (or convertible to integers), the lowest of them is 1, and there are no gaps.
* The table has a metatable whose `__json_is_zero_indexed` property is truthy, the table's pairs are all integers (or convertible to integers), the lowest of them is 0, and there are no gaps.

When tables are serialized as object literals, their keys are sorted, preferring a numeric sort when possible and a string sort otherwise. Keys are coerced to strings, so if two keys have an identical string representation (remember: in Lua, `foo[5] ~= foo["5"]`), then they'll clobber each other.

## `json.parser`

A class which powers the `json.to` function. Takes the JSON string as a constructor argument; call `my_parser:exec()` to parse, and then check `my_parser.result`.