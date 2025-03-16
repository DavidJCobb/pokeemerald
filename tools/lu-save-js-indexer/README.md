
This post-build script dumps specific groups of preprocessor macro definitions from the game's source code (using `gcc -E`), and then analyzes those definitions in order to generate *extra-data files* for use by the JS-based save editor (`lu-save-js`). We attempt to resolve each macro's `#define`d value to an integer constant and, if successful, we store that value and use it both in future substitutions (within other macros) and, in specific cases, as output to the extra-data files.

The goal is to generate a space-optimized (and therefore bandwidth-optimized) collection of data for `lu-save-js` to use for displaying game data, in order to avoid having to hardcode that data into `lu-save-js` itself. For example, rather than hardcoding the names of all game items into the save editor, an extra-data file can be generated which lists all of the `ITEM_` constants from `include/constants/item.h`.

Extra-data files apply to a particular savedata serialization version (i.e. the `SAVEDATA_SERIALIZATION_VERSION` macro in the modified savedata system). For a given version <var>n</var>, they are stored in <code>lu-save-js/formats/<var>n</var>/</code>, using the <code>.dat</code> file extension.

The core post-build logic is stored in this folder, but additional support files are present in `lu-lua-lib`.

## Support libraries

As mentioned, various support libraries are stored in `tools/lu-lua-lib`. I may move them to this folder later.

* **`classes.lua`:** Defines a `make_class` function that can be used to create classes with constructors, single inheritance, and static and instance members. See code comments inside for documentation.

* **`console.lua`:** Not yet used. A singleton that aids with printing output to the console, assuming support for ANSI escape codes.

* **`dataview.lua`:** An analogue to JavaScript's `DataView` class, allowing one to work with sequences of bytes and output them to a file.

* **`shell.lua`:** A wrapper around `os.exec` and similar commands, to make it easier to invoke shell commands.

* **`stdext.lua`:** Extensions to Lua's standard library.

* **`stringify-table.lua`:** Functions to stringify and print the contents of a table. See code comments inside for documentation.

These files are included from `main.lua`.

## Processing macros

Most macros are simple integers, e.g. `#define foo 1`. However, many macros are expressions which produce integer constants, e.g. `#define bar (foo + 3)`. We wish to be able to resolve the latter category of macros to their integer values.

To facilitate this, the Lua library files include a C lexer and expression parser, and a function which attempts to treat a C AST node as an expression tree and compute an integer constant. The parser is based on [the C standard as it existed in a 2007 draft](https://www.open-std.org/JTC1/SC22/WG14/www/docs/n1256.pdf) (see section 6.5). Full parsing of all syntactic elements is not implemented: for example, any non-whitespace character sequence that doesn't match a known symbol is assumed to be an identifier regardless of whether it'd be a valid identifier in C; and typenames are parsed as single identifiers, so `struct Foo` would fail to parse. The parser is best-effort, designed to handle the specific needs of this post-build script; if any macro's value fails to parse, then we assume that it's "real code" and not just a constant value, and we skip loading the macro.

Relevant files:

* `tools/lu-lua-lib/c-ast.lua`
* `tools/lu-lua-lib/c-lex.lua`
* `tools/lu-lua-lib/c-parser.lua`
* `tools/lu-lua-lib/c-exec-integer-constant-expression.lua`


## Extra-data file format

Extra-data files are little-endian binary blobs divided into *subrecords*. Each subrecord has a header prefixing its data:

| Offset | Size | Field | Description |
| -: | -: | :- | :- |
| 0 | 8 | Signature | An eight-CC which identifies the subrecord type. |
| 8 | 4 | Version | The version number. If changes are made to a given subrecord's format, this version number can be used to allow newer readers to tell old-format and new-format subrecords apart. |
| 12 | 4 | Size | The size of the subrecord data, not including the size of this header. |
| 16 | | Data | The subrecord data. Format varies based on the signature. |

### Subrecords

#### `ENUMDATA`

Describes an enumeration that may be of interest to the save editor. The precise format of the data varies depending on whether the enumeration is sparse or contiguous. The basics are that the enum is assumed to have a name prefix shared by all of its members, and that the unprefixed member names are stored in a single block, with sparse enums having all of the values in a block after that.

The start of the subrecord body is always the same (offsets are relative to the start of the subrecord body, not the start of the entire subrecord):

| Offset | Size | Field | Description |
| -: | -: | :- | :- |
| 0 | 1 | Flags | Bit 0 indicates whether this enumeration's values are signed. Bit 1 indicates whether the enumeration is sparse. Bit 2 indicates whether the enumeration's lowest value is non-zero. |
| 1 | 1 | Prefix length | The length of the prefix string. |
| 2 | \* | Prefix string | A sequence of 8-bit ASCII characters: a prefix that applies to all enumeration members. The text is not null-terminated. |
| \* | 4 | Count | The number of members in the enumeration. |

Data after this point varies depending on the enumeration's flags.

* If the enumeration is contiguous and the lowest value is non-zero, then read a 4-byte value and let that be *Lowest*. Otherwise, let *Lowest* be zero.
* If the enumeration is contiguous, then let *NameBlock* be the rest of the subrecord. Otherwise, read a 4-byte unsigned integer *NameBlockSize*: *NameBlock* encompasses the next *NameBlockSize* bytes of the subrecord.
* Read *NameBlock* as a buffer of 8-bit ASCII characters. This block consists of all of the enumeration members' names with the enum's prefix removed, separated with a single null byte. They are not specifically null-terminated: the last name is not followed by a null byte.
  * If the number of names in *NameBlock* does not equal the *Count* value read previously, then the subrecord is ill-formed.
  * If the enumeration is contiguous, then *NameBlock* is sorted in ascending order by value, such that the *n*-th name refers to value *Lowest* + *n*. There is no more data to read.
* If the enumeration is sparse, then let *ValueBlock* be the rest of the subrecord. This block consists of *Count* 4-byte integer values, such that the *n*-th name in *NameBlock* refers to the *n*-th value in *ValueBlock*.

As an example, the `NATURES` enum from the vanilla game is encoded as:

```hex
Offset    00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  Text

00000000  45 4E 55 4D 44 41 54 41 01 00 00 00 AE 00 00 00  ENUMDATA....®...
00000010  00 07 4E 41 54 55 52 45 5F 19 00 00 00 48 41 52  ..NATURE_....HAR
00000020  44 59 00 4C 4F 4E 45 4C 59 00 42 52 41 56 45 00  DY.LONELY.BRAVE.
00000030  41 44 41 4D 41 4E 54 00 4E 41 55 47 48 54 59 00  ADAMANT.NAUGHTY.
00000040  42 4F 4C 44 00 44 4F 43 49 4C 45 00 52 45 4C 41  BOLD.DOCILE.RELA
00000050  58 45 44 00 49 4D 50 49 53 48 00 4C 41 58 00 54  XED.IMPISH.LAX.T
00000060  49 4D 49 44 00 48 41 53 54 59 00 53 45 52 49 4F  IMID.HASTY.SERIO
00000070  55 53 00 4A 4F 4C 4C 59 00 4E 41 49 56 45 00 4D  US.JOLLY.NAIVE.M
00000080  4F 44 45 53 54 00 4D 49 4C 44 00 51 55 49 45 54  ODEST.MILD.QUIET
00000090  00 42 41 53 48 46 55 4C 00 52 41 53 48 00 43 41  .BASHFUL.RASH.CA
000000A0  4C 4D 00 47 45 4E 54 4C 45 00 53 41 53 53 59 00  LM.GENTLE.SASSY.
000000B0  43 41 52 45 46 55 4C 00 51 55 49 52 4B 59 -- --  CAREFUL.QUIRKY
```

Observe the following details:

* The flags byte, here located at offset 0x10, is 0x00. The enum is unsigned, contiguous, and starts at zero.
* The prefix is a length-prefixed string with a single-byte prefix: 0x07; ergo the next seven bytes, `NATURE_`, are the prefix.
* There are 0x19 (24) enum members.
* Each enum member name is serialized without its prefix; so `NATURE_HARDY` is serialized as just `HARDY`.
* There is no null byte after `QUIRKY`, because the names are null-*separated*, not null-*terminated*.

The format is such that you can rapidly extract all enum names in JavaScript by spawning a `TextDecoder`, calling its `decode` method on a `DataView` consisting only of *NameBlock*, and then calling `split("\0")` on the result.

#### `VARIABLS`

Describes individual macros whose values are integer constants. The subrecord consists of zero or more repetitions of the following structure (offsets are relative to the start of the subrecord body, not the start of the entire subrecord):

| Offset | Size | Field | Description |
| -: | -: | :- | :- |
| 0 | 2 | *NameSize* | The number of bytes that make up the macro's name. |
| 2 | *NameSize* | Name | A sequence of 8-bit ASCII characters. No null terminator. |
| 2 + *NameSize* | 1 | Flags | Bit 0 indicates that the value is signed. |
| 3 + *NameSize* | 4 | Value | The macro's four-byte integer value. |