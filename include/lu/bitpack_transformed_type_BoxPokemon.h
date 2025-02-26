#ifndef GUARD_LU_SAVE_TRANSFORMED_TYPE_BoxPokemon
#define GUARD_LU_SAVE_TRANSFORMED_TYPE_BoxPokemon

#include "lu/game_typedefs.h"
#include "global.h"
#include "pokemon.h"

struct SerializedBoxPokemonSubstructs {
   struct PokemonSubstruct0 type0;
   struct PokemonSubstruct1 type1;
   struct PokemonSubstruct2 type2;
   struct PokemonSubstruct3 type3;
   LU_BP_CATEGORY("checksum-16") u16 checksum;
};

struct SerializedBoxPokemon {
   u32   personality;
   u32   otId;
   PokemonNameNoTerminator nickname;
   LanguageID language;
   bool8 isBadEgg   : 1;
   bool8 hasSpecies : 1;
   bool8 isEgg      : 1;
   bool8 blockBoxRS : 1;
   PlayerNameNoTerminator otName;
   u8    markings;
   u16   unknown;
   struct SerializedBoxPokemonSubstructs substructs;
};

#endif