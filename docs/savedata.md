
# Savedata

Pokemon Emerald stores the player's save file to 128KB flash memory. This memory is divided into 4096-byte strips called <dfn>sectors</dfn>. Emerald stores a main save file and a hidden backup save file.


## In the vanilla game

Emerald allocates flash memory sectors as follows:

| ID | Save file | Name | Description |
| :- | :- | :- | :- |
| 0 | 0 | `SECTOR_ID_SAVEBLOCK2` | A single sector which holds the entirety of `SaveBlock2`, dedicated to character and playthrough data. |
| 1 | 0 | `SECTOR_ID_SAVEBLOCK1_START` | `SaveBlock1` is broken across multiple sectors; this is the first of them. |
| ... | 0 | ... | ... |
| 4 | 0 | `SECTOR_ID_SAVEBLOCK1_END` | The last sector allocated to `SaveBlock1`. |
| 5 | 0 | `SECTOR_ID_PKMN_STORAGE_START` | `PokemonStorage` is broken across multiple sectors; this is the first of them. |
| ... | 0 | ... | ... |
| 13 | 0 | `SECTOR_ID_PKMN_STORAGE_END` | The last sector allocated to `PokemonStorage`. |
| 14 | 1 | `SECTOR_ID_SAVEBLOCK2` | A single sector which holds the entirety of `SaveBlock2`, dedicated to character and playthrough data. |
| 15 | 1 | `SECTOR_ID_SAVEBLOCK1_START` | `SaveBlock1` is broken across multiple sectors; this is the first of them. |
| ... | 1 | ... | ... |
| 18 | 1 | `SECTOR_ID_SAVEBLOCK1_END` | The last sector allocated to `SaveBlock1`. |
| 19 | 1 | `SECTOR_ID_PKMN_STORAGE_START` | `PokemonStorage` is broken across multiple sectors; this is the first of them. |
| ... | 1 | ... | ... |
| 27 | 1 | `SECTOR_ID_PKMN_STORAGE_END` | The last sector allocated to `PokemonStorage`. |
| 28 | Both | `SECTOR_ID_HOF_1` | Hall of Fame data requires two sectors and has no backup. |
| 29 | Both | `SECTOR_ID_HOF_2` | Hall of Fame data requires two sectors and has no backup. |
| 30 | Both | `SECTOR_ID_TRAINER_HILL` | Trainer Hill data has no backup. |
| 31 | Both | `SECTOR_ID_RECORDED_BATTLE` | The player's recorded battle has no backup. |

The trick, though, is that Emerald cycles the sectors around; it doesn't always write them to the same place within flash memory. Rather, sectors within a save slot are shifted by one position every time that save slot is updated.

Each saved sector has a 128-byte footer, of which only the last few bytes are actually used. These store the sector ID (so that the game knows which sector it's looking at) and some metadata used to prevent save corruption. This leaves 3968 bytes of data per sector. The game basically just performs a `memcpy` on the to-be-saved data structures, slicing them across sectors at byte boundaries.

## In this branch

We use a bitpacked representation: a value consumes *exactly* as many bits as it needs. If a value will never be greater than 7, then it only needs 4 bits, even if it's a `u8` in memory.

A custom compiler plug-in (`lu-bitpack`) generates this bitpacked representation automatically, along with the code to convert to and from it. This means that we can't predict exactly how many sectors a given piece of savedata will consume, nor can we predict how sectors are allocated. We have a few strategies to deal with this:

* The compiler plug-in allows us to set the max sector size and the max amount of sectors, so we know we won't blow past the 14 sectors allotted per save slot.
* The compiler plug-in allows us to force a given piece of savedata into its own sector.
* The compiler plug-in has `#pragma` directives that can be used to define a new variable, whose value is the sector ID of some piece of savedata.

These things in combination allow us to write code that needs to know what sectors a given piece of savedata has ended up in. Our code assumes that the relative ordering of savedata is unchanged:

* Character and playthrough data (`SaveBlock2`)
* World state (`SaveBlock1`)
* Pokemon Storage System (`PokemonStorage`)

This assumption is relevant for parts of the game that write savedata either partially or through latent functions:

* Some parts of the game want to update everything except the Pokemon Storage System. (This is the cause of the infamous Battle Frontier cloning exploit.)
* Saves made as part of a multiplayer link will update everything except the Pokemon Storage System, *and* they'll do it in a latent manner: `SaveBlock2` is written all at once (on the assumption that it consumes only a single sector), and `SaveBlock1` is written one sector per frame.
  * Yes, this means that every individual multiplayer feature reaches into the guts of the save system. Yes, that makes it difficult to refactor things in a safe and sane way. Our current refactor uses a `for`-loop for `SaveBlock2` in case it grows to cover more than a single sector, while keeping the latent mechanism (based on a global counter) for `SaveBlock1`.
