
This post-build script dumps specific groups of preprocessor macro definitions from the game's source code (using `gcc -E`), and then analyzes those definitions in order to generate *extra-data files* for use by the JS-based save editor (`lu-save-js`). We attempt to resolve each macro's `#define`d value to an integer constant and, if successful, we store that value and use it both in future substitutions (within other macros) and, in specific cases, as output to the extra-data files.

The goal is to generate a space-optimized (and therefore bandwidth-optimized) collection of data for `lu-save-js` to use for displaying game data, in order to avoid having to hardcode that data into `lu-save-js` itself. For example, rather than hardcoding the names of all game items into the save editor, an extra-data file can be generated which lists all of the `ITEM_` constants from `include/constants/item.h`.

Extra-data files apply to a particular savedata serialization version (i.e. the `SAVEDATA_SERIALIZATION_VERSION` macro in the modified savedata system). For a given version <var>n</var>, they are stored in <code>lu-save-js/formats/<var>n</var>/</code>, using the <code>.dat</code> file extension.

The core post-build logic is stored in this folder, but additional support files are present in `lu-lua-lib`.

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

#### `ENUNUSED`

Represents ranges of unused (as opposed to unspecified) values within an enum. This is principally used with the `FLAG` enum, to avoid serializing hundreds of `FLAG_UNUSED_0x...` names into the `ENUMDATA` subrecords while still being able to have the save editor explicitly list these flags as unused (rather than unrecognized).

The data is as follows:

* Length-prefixed string (one-byte length) indicating the enum prefix (same as `ENUMDATA`)
* Zero or more ranges, consisting of...
  * Four-byte enum value: the first value in this range
  * Four-byte enum value: the last value in this range

Consider the following example:

```hex
Offset    00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  Text

00000000  45 4E 55 4E 55 53 45 44 01 00 00 00 86 00 00 00  ENUNUSED....†...
00000010  05 46 4C 41 47 5F 20 00 00 00 4F 00 00 00 54 00  .FLAG_ ...O...T.
00000020  00 00 55 00 00 00 68 00 00 00 68 00 00 00 E9 00  ..U...h...h...é.
00000030  00 00 E9 00 00 00 AA 01 00 00 AB 01 00 00 DA 01  ..é...ª...«...Ú.
00000040  00 00 DA 01 00 00 DE 01 00 00 E3 01 00 00 64 02  ..Ú...Þ...ã...d.
00000050  00 00 BB 02 00 00 D9 02 00 00 D9 02 00 00 68 04  ..»...Ù...Ù...h.
00000060  00 00 68 04 00 00 70 04 00 00 70 04 00 00 72 04  ..h...p...p...r.
00000070  00 00 72 04 00 00 79 04 00 00 79 04 00 00 93 04  ..r...y...y...“.
00000080  00 00 EF 04 00 00 F9 04 00 00 FA 04 00 00 FF 04  ..ï...ù...ú...ÿ.
00000090  00 00 FF 04 00 00                                ..ÿ...
```

We see first the length-prefixed string: length `05`, with content `FLAG_`. Immediately following this is our first range: [0x20, 0x4F]; all enum values that are >= 0x20 and <= 0x4F are unused. Then, the next range: [0x54, 0x55]: two unused values in a row. Then, the next range: [0x68, 0x68]: a lone unused value in the middle of several used values. And on it goes.

`ENUNUSED` should generally appear alongside (ideally after) an `ENUMDATA`, and so the signedness of the values in the ranges depends on the signedness of the enum.

#### `MAPSDATA`

This data is generated from the `map_groups.json` file, and is used by the save editor to identify map groups and map numbers.

The subrecord is divided into multiple *prefixed name blocks*. Each prefixed name block consists of:

* A length-prefixed string (one-byte length). This is a prefix to be applied to all names in the block.
* The size of the block data, as an unsigned 32-bit integer.
* The block data: a blob of null-*separated* (*not* null-*terminated*) strings.

The subrecord is arranged as follows:

* A prefixed name block for the map group names.
* For each map group, a prefixed name block for the map names therein.

As an example, here's the start of a `MAPSDATA` subrecord for the vanilla game:

```hex
Offset(h) 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  Text

00000000  4D 41 50 53 44 41 54 41 01 00 00 00 DA 26 00 00  MAPSDATA....Ú&..
00000010  0A 67 4D 61 70 47 72 6F 75 70 5F 06 02 00 00 54  .gMapGroup_....T
00000020  6F 77 6E 73 41 6E 64 52 6F 75 74 65 73 00 49 6E  ownsAndRoutes.In
00000030  64 6F 6F 72 4C 69 74 74 6C 65 72 6F 6F 74 00 49  doorLittleroot.I
00000040  6E 64 6F 6F 72 4F 6C 64 61 6C 65 00 49 6E 64 6F  ndoorOldale.Indo
00000050  6F 72 44 65 77 66 6F 72 64 00 49 6E 64 6F 6F 72  orDewford.Indoor
00000060  4C 61 76 61 72 69 64 67 65 00                    Lavaridge.
```

Here, we see the start of the prefixed name block for map group names. Inside, the prefix string is `gMapGroup_`. The first few map group names are `gMapGroup_TownsAndRoutes` and `gMapGroup_IndoorLittleroot`, but you can see that when the subrecord was serialized, the common prefix was separated and omitted: the map group names were serialized as `TownsAndRoutes` and `IndoorLittleroot`.

#### `VARIABLS`

Describes individual macros whose values are integer constants. The subrecord consists of zero or more repetitions of the following structure (offsets are relative to the start of the subrecord body, not the start of the entire subrecord):

| Offset | Size | Field | Description |
| -: | -: | :- | :- |
| 0 | 2 | *NameSize* | The number of bytes that make up the macro's name. |
| 2 | *NameSize* | Name | A sequence of 8-bit ASCII characters. No null terminator. |
| 2 + *NameSize* | 1 | Flags | Bit 0 indicates that the value is signed. |
| 3 + *NameSize* | 4 | Value | The macro's four-byte integer value. |