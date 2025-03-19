# Pokémon Emerald

This is a fork of a decompilation of Pokémon Emerald.


## Original PRET Info

To set up the repository, see [INSTALL.md](INSTALL.md).

For contacts and other pret projects, see [pret.github.io](https://pret.github.io/).


## This fork

This fork uses a custom compiler plug-in to generate code that can pack Emerald's save files in a format which is optimized for space.


### New dependencies

* **`libgmp3-dev`:** Needed in order to compile GCC plug-ins. A GCC plug-in must be compiled for the specific instance of GCC (i.e. version, host architecture, and target architecture) that you intend to use the plug-in with.

* **`lua5.4`:** Used for post-build scripts. Installable via `sudo apt install lua5.4`.


### Code changes

* Everything in the `plugins` folder is new.

* `Makefile` has been altered: the command-line parameters used for the compiler now specify the compiler plug-ins to use.

* You are now meant to compile Emerald by running `bash build.sh`. This shell script invokes the makefile for all extant compiler plug-ins, checking and erroring on any missing dependencies (recommending commands to install them), before then invoking the makefile for Emerald and the post-build scripts.

* Source files have been added or edited as necessary to define bitpacking options for savedata.

  * Added
  
    * `include/lu/bitpack_options.h`: Convenience macros for setting bitpacking options as attributes on fields and types.
    
    * `include/lu/bitpack_transform_BoxPokemon.h`<br>`src/lu/bitpack_transform_BoxPokemon.c`: Declares the `SerializedBoxPokemon` type and the functions which convert between it and `BoxPokemon`.
    
    * `include/lu/bitpack_transform_type_BoxPokemon.h`: Defines the `SerializedBoxPokemon` type.
  
    * `include/lu/bitstream.h`<br>`src/lu/bitstream.c`: Defines a type and functions for working with streams of non-byte-aligned data. The generated code will invoke these functions.
    
    * `include/lu/game_typedefs.h`: Defines several `typedef`s which have bitpacking options set on them, e.g. `PokemonLevel` and `MoveID`, so that these options don't have to be set in every individual place where a Pokemon level (and various other common types) may appear.
    
    * `include/save_encryption.h` and `src/save_encryption.c`: Contains functions originally in `src/load_save.c`, so we can access them from within our modified savegame code.
    
    * `src/save-codegen.h`: Included by `save.c` at the appropriate time in order to trigger generation of the bitpacking code.
    
    * `src/save-codegen-options.h`: Included by `save.c` at the appropriate time in order to set options that apply to the overall code generation process, such as which variables to generate bitpacking code for.
  
  * Edited
    
    * `include/global.h`: Edited to set bitpacking options on various fields, including by using the typedefs defined in `include/lu/game_typedefs.h`.
  
    * `include/global.berry.h`: Edited to set bitpacking options on Enigma Berry savedata fields.
  
    * `include/global.tv.h`: Edited to set bitpacking options on various fields, including by using the typedefs defined in `include/lu/game_typedefs.h`.
    
    * `src/load_save.c`: Edited to move `ApplyNewEncryptionKeyToBagItems` to `include/save_encryption.h`.
  
    * `include/pokemon.h`: Edited to set bitpacking options on various fields, including by using the typedefs defined in `include/lu/game_typedefs.h`.
  
    * `include/pokemon_storage_system.h`: Edited to set bitpacking options on various fields, including by using the typedefs defined in `include/lu/game_typedefs.h`.

* `src/save.c` has been rewritten to invoke the generated code.

* `src/save.h` has been altered to remove macros that are no longer meaningful (i.e. macros whose proper values are no longer knowable without invoking code generation). Additionally, a `version` field has been added to the save sector footer.

* New tools have been added to the `tools` folder.

  * **`lu-save-js`:** A web-based editor for bitpacked saves, using the data format produced when compiling the game.
  
  * **`lu-save-js-indexer`:** Post-build scripts written in Lua, which generate important data files for the save editor. These data files allow the save editor to understand things like Pokemon species names without that data being hardcoded into the save editor; it can be generated from the game's source code.
  
  * **`lu-save-report-generator`:** A post-build script written in Lua, which analyzes the XML file produced by the bitpack plug-in in order to produce a human-readable report on how much space has been saved.
  
  * **`lu-lua-lib`:** Lua libraries used by the post-build scripts.