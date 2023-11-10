# Pokémon Emerald with savegame bitpacking

In vanilla Emerald, the `SaveBlock1` (world state) and `SaveBlock2` (character state) structs consume 99% of the space allotted to them in flash memory (savedata). This is because they are blindly `memcpy`'d from RAM. A bitpacked format would consume substantially less space. However, maintaining the code to bitpack these structs would be prohibitively difficult by hand because savedata is split into ~4KiB strips ("sectors"); you'd have to pause serialization mid-struct &mdash; potentially within some deeply-nested struct or array hierarchy &mdash; and resume within the next sector. This branch uses [custom code generation](https://github.com/DavidJCobb/pokeemerald-savedata-codegen) to produce the serialization code while properly minding sector boundaries.

The following changes are made to Emerald itself:

* The makefile is adjusted to allow for deeper folder hierarchies for `*.h` and `*.c` files.
* `item.c` is modified so that saving item quantities as 10-bit numbers doesn't break the game's encryption of them (i.e. the six truncated bits are lopped off during the re-encryption step).
* `save.h` and `save.c` are rewritten. The original code mapped RAM regions to sector IDs, whereas the rewrite maps sector IDs to functions that perform reading and writing of bitpacked data.
   * A lot of code has been de-duplicated as well. I haven't tested what effect this will have on multiplayer link saves.
* Tons of struct definitions in `global.h`, `pokemon.h`, and other files have been modified to `#include` generated struct member lists. The goal is to ensure a "single source of truth" for the contents of all savegame-relevant structs, save for those that the generator can't actually produce yet (i.e. named union types).
* My bitstream functions, for reading and writing bitpacked data, are included in `lu/bitstreams.h` and `lu/bitstreams.c`.
* Generated code can be found in `lu/generated/`.

XML files used for the purposes of codegen are stored in the `lu-data/codegen-xml/` directory. In general, "Lu" is used as as prefix for most of the changed content. **Code generation is not part of the normal build process, since that runs in Linux and the code generator is built for Windows. If you modify anything that gets generated, you need to re-run the code generator manually.**

In tests, early-game savedata seems to save and load properly. Late-game savedata is not tested, though the late-game data looks pretty normal when using the codegen system's built-in save dumper to analyze an early savegame. We don't dump special sectors (Hall of Fame; Trainer Hill; Recorded Battle), though, so I can't promise that I haven't damaged those somehow. Maybe I'll test those in-game using a walk-through-walls code at some point.

I don't know what software license to put on this *entire* package, but, like, I'm not gonna call the Internet police on you if you use it for your own Pokémon modding projects.

Original README follows.



# Pokémon Emerald

This is a decompilation of Pokémon Emerald.

It builds the following ROM:

* [**pokeemerald.gba**](https://datomatic.no-intro.org/index.php?page=show_record&s=23&n=1961) `sha1: f3ae088181bf583e55daf962a92bb46f4f1d07b7`

To set up the repository, see [INSTALL.md](INSTALL.md).


## See also

Other disassembly and/or decompilation projects:
* [**Pokémon Red and Blue**](https://github.com/pret/pokered)
* [**Pokémon Gold and Silver (Space World '97 demo)**](https://github.com/pret/pokegold-spaceworld)
* [**Pokémon Yellow**](https://github.com/pret/pokeyellow)
* [**Pokémon Trading Card Game**](https://github.com/pret/poketcg)
* [**Pokémon Pinball**](https://github.com/pret/pokepinball)
* [**Pokémon Stadium**](https://github.com/pret/pokestadium)
* [**Pokémon Gold and Silver**](https://github.com/pret/pokegold)
* [**Pokémon Crystal**](https://github.com/pret/pokecrystal)
* [**Pokémon Ruby and Sapphire**](https://github.com/pret/pokeruby)
* [**Pokémon Pinball: Ruby & Sapphire**](https://github.com/pret/pokepinballrs)
* [**Pokémon FireRed and LeafGreen**](https://github.com/pret/pokefirered)
* [**Pokémon Mystery Dungeon: Red Rescue Team**](https://github.com/pret/pmd-red)
* [**Pokémon Diamond and Pearl**](https://github.com/pret/pokediamond)
* [**Pokémon Platinum**](https://github.com/pret/pokeplatinum) 
* [**Pokémon HeartGold and SoulSilver**](https://github.com/pret/pokeheartgold)
* [**Pokémon Mystery Dungeon: Explorers of Sky**](https://github.com/pret/pmd-sky)

## Contacts

You can find us on:

* [Discord (PRET, #pokeemerald)](https://discord.gg/d5dubZ3)
* [IRC](https://web.libera.chat/?#pret)
