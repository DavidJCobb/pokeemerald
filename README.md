# Pokémon Emerald

This is a fork of a decompilation of Pokémon Emerald.


## Original PRET Info

To set up the repository, see [INSTALL.md](INSTALL.md).

For contacts and other pret projects, see [pret.github.io](https://pret.github.io/).


## This fork

This fork uses a custom compiler plug-in to generate code that can pack Emerald's save files in a format which is optimized for space. The following changes have been made:

* Everything in the `plugins` folder is new.

* `Makefile` has been altered: the command-line parameters used for the compiler now specify the compiler plug-ins to use.

* You are now meant to compile Emerald by running `bash build.sh`. This shell script invokes the makefile for all extant compiler plug-ins, including any shell scripts needed to set up their dependencies, before then invoking the makefile for Emerald itself.

* Source files have been added or edited as necessary to define bitpacking options for savedata.

  * Added
  
    * `include/lu/bitpack_options.h`: Convenience macros for setting bitpacking options as attributes on fields and types.
    
    * `include/lu/bitpack_transform_BoxPokemon.h`<br>`src/lu/bitpack_transform_BoxPokemon.c`: Declares the `SerializedBoxPokemon` type and the functions which convert between it and `BoxPokemon`.
    
    * `include/lu/bitpack_transform_type_BoxPokemon.h`: Defines the `SerializedBoxPokemon` type.
  
    * `include/lu/bitstream.h`<br>`src/lu/bitstream.c`: Defines a type and functions for working with streams of non-byte-aligned data. The generated code will invoke these functions.
    
    * `include/lu/game_typedefs.h`: Defines several `typedef`s which have bitpacking options set on them, e.g. `PokemonLevel` and `MoveID`, so that these options don't have to be set in every individual place where a Pokemon level (and various other common types) may appear.
    
    * `src/save-codegen.h`: Included by `save.c` at the appropriate time in order to trigger generation of the bitpacking code.
    
    * `src/save-codegen-options.h`: Included by `save.c` at the appropriate time in order to set options that apply to the overall code generation process, such as which variables to generate bitpacking code for.
  
  * Edited
    
    * `include/global.h`: Edited to set bitpacking options on various fields, including by using the typedefs defined in `include/lu/game_typedefs.h`.
  
    * `include/global.berry.h`: Edited to set bitpacking options on Enigma Berry savedata fields.
  
    * `include/global.tv.h`: Edited to set bitpacking options on various fields, including by using the typedefs defined in `include/lu/game_typedefs.h`.
  
    * `include/pokemon.h`: Edited to set bitpacking options on various fields, including by using the typedefs defined in `include/lu/game_typedefs.h`.
  
    * `include/pokemon_storage_system.h`: Edited to set bitpacking options on various fields, including by using the typedefs defined in `include/lu/game_typedefs.h`.

* `src/save.c` has been rewritten to invoke the generated code.

* `src/save.h` has been altered to remove macros that are no longer meaningful (i.e. macros whose proper values are no longer knowable without invoking code generation). Additionally, a `version` field has been added to the save sector footer.