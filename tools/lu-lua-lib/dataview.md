
# `dataview`

An analogue to JavaScript's `DataView`. The `dataview` object is a class (with a dependency on the `classes` library).

## Getters

### `size`

Returns the bytecount of the dataview.


## Member functions

### `view:append_eight_cc(value)`

Appends an eight-character code (a string) to the end of the dataview, extending its length.

### `view:append_four_cc(value)`

Appends a FourCC (four-character string) to the end of the dataview, extending its length.

### `view:append_length_prefixed_string(prefix_width, value, big_endian_prefix)`

Appends a length-prefixed string to the end of the dataview, extending the dataview's length.

The length prefix is `prefix_width` bytes long; if `big_endian_prefix` is truthy, then the prefix is written as a big-endian value. The string's character data is written immediately after the prefix.

### `view:append_raw_string(value)`

Appends the content of a string to the end of the dataview, extending its length.

### `view:append_uint8(value)`

Appends an 8-bit (1-byte) unsigned value to the end of the dataview, extending its length.

A negative value will have 255 added to it to simulate overflow to positive. The function raises an error if the value (after this addition, if it's performed) is not representable in eight bits.

### `view:append_uint16(value, big_endian)`

Appends an 16-bit (2-byte) unsigned value to the end of the dataview, extending its length. Writes the value as big-endian if the `big_endian` parameter is truthy.

A negative value will have 65535 added to it to simulate overflow to positive. The function raises an error if the value (after this addition, if it's performed) is not representable in sixteen bits.

### `view:append_uint32(value, big_endian)`

Appends an 32-bit (4-byte) unsigned value to the end of the dataview, extending its length. Writes the value as big-endian if the `big_endian` parameter is truthy.

### `view:get_uint8(offset)`

Returns the 8-bit (1-byte) unsigned integer at the given `offset`. Raises an error if the offset is out of bounds.

### `view:get_uint16(offset, big_endian)`

Returns the 16-bit (2-byte) unsigned integer at the given `offset`. Reads the value as big-endian if the `big_endian` parameter is truthy. Raises an error if the offset is out of bounds.

### `view:get_uint32(offset, big_endian)`

Returns the 32-bit (4-byte) unsigned integer at the given `offset`. Reads the value as big-endian if the `big_endian` parameter is truthy. Raises an error if the offset is out of bounds.

### `view:is_identical_to(other)`

Returns a boolean indicating whether this dataview is of the same size as, and has identical bytes to, `other`.

### `view:load_from_file(path)`

Opens the specified file (via `io.open(path, "rb")`) and then reads its content into the dataview. The dataview is cleared first.

### `view:print(octets_per_row)`

Prints a hex dump of this dataview's contents, with three columns: the offset of each row, the bytes, and a column displaying the bytes interpreted as ASCII (with periods substituted in for non-printable characters).

The `octets_per_row` parameter is optional; if specified, it controls the number of bytes (and by extension, chars in the text column) per row.

### `view:resize(size)`

Resizes the dataview to `size` many bytes. If the provided `size` is larger, then the new bytes are initialized to zero.

### `view:save_to_file(path)`

Opens the specified file (via `io.open(path, "wb")`) and writes the dataview's content to it.

### `view:set_uint8(offset, value)`

Writes an 8-bit (1-byte) unsigned value to the specified offset.

A negative value will have 255 added to it to simulate overflow to positive. The function raises an error if the value (after this addition, if it's performed) is not representable in eight bits, or if the offset is out of bounds.

### `view:set_uint16(offset, value)`

Writes an 16-bit (2-byte) unsigned value to the specified offset.

A negative value will have 65535 added to it to simulate overflow to positive. The function raises an error if the value (after this addition, if it's performed) is not representable in eight bits, or if the offset is out of bounds.

### `view:set_uint32(offset, value)`

Writes an 32-bit (4-byte) unsigned value to the specified offset.

The function raises an error if the offset is out of bounds.


## Implementation notes

A lot of code online encourages the use of strings to represent binary data in Lua. Lua itself encourages this by offering functions like `string.char`, `string.pack`, `string.packsize`, and `string.unpack`. However, Lua also uses immutable strings and string interning, which means that strings are a highly inefficient way to construct binary data incrementally: every time you append to the binary data, the entire blob has to be hashed and interned, slowing down Lua's string handling over time.

To avoid these issues, a `dataview` instance uses an array of bytes (i.e. integers) under the hood. Lua's file I/O can only operate via strings, so the `dataview` member functions for reading and writing files operate one byte at a time, converting between ints and chars.
