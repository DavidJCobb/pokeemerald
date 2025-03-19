
# Game typedefs

These are convenience typedefs that are defined here in order to avoid having to repeat bitpacking options all over the place, and in an attempt to help make it easier to adjust certain commonly-used values or types.

## `BattleFacilityLvlMode`

Level modes for both the Battle Frontier facilities and the Battle Tents. The `FRONTIER_LVL_...` constants are valid values for this type.

## `BattleFrontierLvlMode`

Level modes for the Battle Frontier facilities only, excluding the Battle Tents. The `FRONTIER_LVL_...` constants (besides `...TENT`) are valid values for this type.

## `BattleFrontierLvlModeOptional`

Level modes for the Battle Frontier facilities only, but allowing the use of 0 as a "none" value. Non-none values should be `FRONTIER_LVL_...` constants increased by 1, with `FRONTIER_LVL_TENT` disallowed.

## `ContestCategory`

Contest categories, constrained to the range \[0, `CONTEST_CATEGORIES_COUNT`). The `CONTEST_CATEGORY_...` constants are valid values for this type.

## `EasyChatWordID`

The unique (i.e. fully-qualified) ID of an Easy Chat word.

Each value is the combination of an Easy Chat word category and an index within that category, with the lower `EC_MASK_BITS` bits indicating the word and the upper bits indicating the category. Categories, words, and related constants are defined in `include/constants/easy_chat.h`.

## `LanguageID`

Language IDs. These are generally stored alongside text strings whenever the string content may have come from a separate game. The `LANGUAGE_...` constants are valid values for this type.

Within the bitpacked file format, these values are constrained both to the range \[0, `NUM_LANGUAGES`) and to 4 bits. The latter constraint is applied because the following struct fields store languages as 4-bit bitfields:

* `DaycareMail::gameLanguage`
* `DaycareMail::monLanguage`

If you wish to support more than 15 defined languages, then you must adjust those bitfields and then remove the explicitly-set bitcount from the `LanguageID` type.

## `ItemIDGlobal`

Item IDs.

The word "global" here refers to plans for a future optimization. Currently, all items share a common ID space, but this makes it harder to add certain types of items (particularly Poke Ball types), and it results in the player's inventory being bitpacked in a less efficient format than is theoretically possible. In the future, I'd like to develop a compiler plug-in which scans over the list of all items in the game and generates multiple ID spaces separated by bag pocket, along with functions to convert item IDs between the "global" item ID space and these per-pocket ID spaces.

## Map-related typedefs

The game organizes maps through multiple means.

* A **map group** is a collection of related maps, generally associated with a common location. For example, the "towns and routes" map group is used to organize most (all?) outdoor locations, and each city has its own map group to organize its interiors.
* A **map num** is the index of a map within a map group.
* A **map section** is a more "semantic" identifier for maps; broadly, these correspond to names on the Town Map and to Met Locations in Pokemon data.

### `MapGroupIndex` and `MapGroupIndexOptional`

These refer to a map group. The "optional" type encodes an absent map group as `-1`.

### `MapIndexInGroup` and `MapIndexInGroupOptional`

These refer to a map num. The "optional" type encodes an absent map num as `-1`.

When our save editor sees a value which uses one of these typedefs, it checks to see if the value's containing `struct` has exactly one field of type `MapGroupIndex` or `MapGroupIndexOptional`. If so, it assumes that the map-num value refers to a map within the group indicated by the map-group value:

```c
struct Somewhere {
   MapGroupIndex   foo;
   MapIndexInGroup bar;
};
// `bar` is detected as belonging to the `foo` map group.
```

If there are multiple map-group sibling fields, however, then the save editor won't be able to identify which one corresponds to any given map-num without some help via bitpacking misc. annotations.

```c
struct TwoPlaces {
   MapGroupIndex group1;
   LU_BP_ANNOTATION("map-group(group1)") MapIndexInGroup num1;
   MapGroupIndex group2;
   LU_BP_ANNOTATION("map-group(group2)") MapIndexInGroup num2;
};
```

It is recommended that you use these annotations preemptively whenever map groups and map nums appear in any structs that are treated as dumping groups for unsorted data, e.g. `SaveBlock1`.

### `MapSectionID`

These refer to map section IDs, excluding any sentinel values that are used within Met Locations exclusively. The `MAPSEC_...` constants are valid values for this type.

Met locations use the same values as map section IDs, with additional sentinel values present; so if you wish to significantly expand the number of map section IDs available (e.g. by changing the value to a `u16`), then be mindful of the `METLOC_...` constants and ensure that your additions to `MAPSEC_...` don't conflict with any of them.

### `MetLocation`

These refer to Met Locations, including both map section IDs and special sentinel values. The `MAPSEC_...` and `METLOC_...` constants are valid values for this type.

## `MoveID`

These refer to moves that Pokemon can learn.

Within the bitpacked file format, this value is constrained to 9 bits for consistency with struct `LevelUpMove` defined in `include/pokemon.h`, which serves as the list entry type for a Pokemon species' list of level-up moves. A nine-bit value can hold move IDs in the range [0, 1023].

As of Generation IX, there are 934 moves in the core-series games, including all G-Max moves and Z-Moves, but *not* distinguishing between physical and special variants of a single Z-Move. A ROM hack that needs to support a hypothetical Gen X and onward (i.e. a hack that needs more than 1023 moves) will need to expand the width of the `move` bitfield on `LevelUpMove`, and then adjust the `MoveID` typedef to match.

## `PlayerGender`

These refer to player-characters' genders, i.e. `MALE` or `FEMALE`, with the maximum determined by `GENDER_COUNT`.

## `PlayerName` and `PlayerNameNoTerminator`

These are player names. Different typedefs exist to indicate whether the string always has a terminator byte while in memory (and therefore takes an extra byte of RAM). The lengths of these types are based on the `PLAYER_NAME_LENGTH` constant in `include/constants/global.h`.

Note that the Mauvile Trader (`MauvilleOldManTrader`) uses a different type to store player names, with length `PLAYER_NAME_LENGTH + 4 == 11`. This is because Ruby and Sapphire didn't store `LanguageID` values for the player names he remembered; instead, they treated Latin-encoded languages as the default, and wrapped the stored player names in control codes for Japanese players. That is: `"Annabel"`, when transferred to a Japanese Ruby and Sapphire copy, would be stored as `EXT_CTRL_CODE_BEGIN EXT_CTRL_CODE_ENG 'A' 'n' 'n' 'a' 'b' 'e' 'l' EXT_CTRL_CODE_BEGIN EXT_CTRL_CODE_JPN` without a terminator byte. Emerald does not do this, and will convert the Mauville Trader data between legacy and modern formats during record mixing; but it still reserves the same number of bytes for player names, likely so that the same `OldMan` union and `MauvilleOldManTrader` struct can be used both for locally stored data and when setting up data for transfer during record mixing. Additionally, Emerald takes advantage of the empty space by assuming that the locally stored string is always null-terminated, even though the string is potentially unterminated in the legacy format and (I certainly hope!) would be treated as such by Japanese Ruby and Sapphire.

## Pokemon data

### `PokemonLevel`

The level of a Pokemon, limited to the range [0, `MAX_LEVEL`]. The `MAX_LEVEL` constant is defined in `include/constants/pokemon.h`, with value 100 in the vanilla game.

### `PokemonName` and `PokemonNameNoTerminator`

These are Pokemon names or nicknames. Different typedefs exist to indicate whether the string always has a terminator byte while in memory (and therefore takes an extra byte of RAM). The lengths of these types are based on the `POKEMON_NAME_LENGTH` constant in `include/constants/global.h`.

### `PokemonPersonality`

These values act as the "DNA" for a Pokemon, and are used to compute the following aspects of a Pokemon:

* Ability
* Gender
* Mirage Island appearance trigger
* Nature
* Shininess
* Size (for scripted NPCs that measure Pokemon sizes)
* Species-specific special cases
  * Egg species, for Pokemon that use one species per gender (Nidoran; Volbeat/Illumise)
  * Spinda spot positions
  * Wurmple's evolution species
  * Unown letter

The vanilla calculations are as follows:

| Metric | Computation | Description |
| :- | :- | :- |
| Ability | `p & 1` | If a species has multiple possible abilities, this metric indicates which ability it uses. |
| Gender | `(p & 0xFF) < g` | If a species isn't gender-locked, then `g` is its gender ratio. If this condition passes, the Pokemon is female; otherwise, male. |
| Mirage Island | `(p & 0xFFFF) == m` | Mirage Island appears if this check passes for any Pokemon in the player's party, given a randomly-generated daily value `m`. |
| Nature | `p % NATURE_COUNT` | There are 24 natures in the vanilla game. |
| Shininess | `GET_SHINY_VALUE(otId, p) >= SHINY_ODDS` | See `include/pokemon.h`. The `GET_SHINY_VALUE` macro splits the Pokemon's personality value and the ID of its original trainer (including hidden digits) into 16-bit values and mashes them together. |
| Size | | See `src/pokemon_size_record.c`. The low two bytes of the personality are mashed together with the Pokemon's IVs to compute a "size hash," which is used to look up size variations. |
| Egg species | `(p >> 15) & 1` | In Gen III only, the personality value of an egg produced by Nidoran♀ or Illumise indicates whether the egg will hatch into the male-counterpart species. Not used if Ditto acts as the female in the breeding pair (e.g. not used for eggs resulting from a Ditto/Nidoran♂ pairing). |
| Spinda spots | `(p >> (8 * n)) & 0xF - 8`<br/>`((p >> (8 * n)) & 0xF0) >> 4 - 8` | Each Spinda spot uses a single byte of the personality value, starting from the least-significant byte. The bytes are divided into nybbles, and each is treated as an offset (in the range [-8, 7]) from the spot's initial position. |
| Wurmple evolution | `((p >> 16) % 10) > 4` | Wurmple evolves into Silcoon if this expression is true, or Cascoon otherwise. Each evolution checks the same threshold. |
| Unown letter | `GET_UNOWN_LETTER(p)` | Defined in `include/pokemon.h`. Combines the lowest two bits of every constituent byte of the personality value. |

### `PokemonSpeciesID`

A Pokemon's species.

Within the bitpacked file format, we constrain these values to the range [0, 2047]. As of March 2025, there are 1025 conventional species of Pokemon in Generations I through IX, including Ultra Beasts, Paradox Pokemon, and DLC additions. Additionally, there are 48 Mega Evolutions.

## `TrainerID`

Trainer IDs have multiple representations throughout savedata and the game's codebase, and I haven't fully identified all of them. This typedef applies to one of the more common and straightforward representations: byte arrays capable of holding a full trainer ID without truncation.

