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
   
   // don't annotate this as a checksum. it requires special logic to handle 
   // in the JS-based save editor, since we don't pad the substructs above to 
   // consistent lengths, so annotating it as a checksum has no benefit
   u16 checksum;
};

struct LU_BP_ZERO_FILL_IF_NEW SerializedBoxPokemon {
   PokemonPersonality personality;
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