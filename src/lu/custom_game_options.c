#include "global.h"
#include "lu/custom_game_options.h"

#include "lu/bitstreams.h"

#define CUSTOM_GAME_OPTION_DATA_VERSION 0

#define BITS_FOR_POKEMON_SPECIES 10

#define STARTER_TRIO_TOTAL_BITS       (BITS_FOR_POKEMON_SPECIES * 3)
#define SINGLE_SPECIES_RUN_TOTAL_BITS (BITS_FOR_POKEMON_SPECIES * 2 + 1 + 1)
#define SINGLE_SPECIES_RUN_SPARE_BITS SINGLE_SPECIES_RUN_TOTAL_BITS - STARTER_TRIO_TOTAL_BITS

void ResetCustomGameOptions(void) {
   gCustomGameOptions.allow_changing_these_during_play = FALSE;
   
   gCustomGameOptions.qol.start_with_running_shoes = FALSE;
   
   gCustomGameOptions.starter_mode = FALSE;
   gCustomGameOptions.starter_trio[0] = 0;
   gCustomGameOptions.starter_trio[1] = 0;
   gCustomGameOptions.starter_trio[2] = 0;
   
   gCustomGameOptions.double_battles = CustomGame_DoubleBattles_Normal;
   
   gCustomGameOptions.nuzlocke.enabled = FALSE;
   gCustomGameOptions.nuzlocke.encounter_limit.type = 0;
   gCustomGameOptions.nuzlocke.encounter_limit.gift_behavior = 0;
   gCustomGameOptions.nuzlocke.encounter_limit.shiny_exception_base = 0;
   gCustomGameOptions.nuzlocke.encounter_limit.dupes_clause.enabled = 0;
   gCustomGameOptions.nuzlocke.encounter_limit.dupes_clause.shiny_exception = 0;
}

void BitWriteCustomGameOptions(CustomGameOptions* cgo, lu_BitstreamState* state) {
   lu_BitstreamWrite_u16(&state, CUSTOM_GAME_OPTION_DATA_VERSION, 16);
   
   lu_BitstreamWrite_bool(&state, cgo.allow_changing_these_during_play);
   
   lu_BitstreamWrite_bool(&state, cgo.qol.start_with_running_shoes);
   
   lu_BitstreamWrite_bool(&state, cgo.starter_mode);
   switch (cgo.starter_mode) {
      case 0:
         lu_BitstreamWrite_u16(&state, cgo.starter_trio[0], BITS_FOR_POKEMON_SPECIES);
         lu_BitstreamWrite_u16(&state, cgo.starter_trio[1], BITS_FOR_POKEMON_SPECIES);
         lu_BitstreamWrite_u16(&state, cgo.starter_trio[2], BITS_FOR_POKEMON_SPECIES);
         break;
      case 1:
         lu_BitstreamWrite_u16(&state, cgo.single_species.player_species, BITS_FOR_POKEMON_SPECIES
         lu_BitstreamWrite_u16(&state, cgo.single_species.rival_base_species, BITS_FOR_POKEMON_SPECIES);
         lu_BitstreamWrite_u8(&state, cgo.single_species.catch_mode, 1);
         lu_BitstreamWrite_bool(&state, cgo.single_species.give_new_mon_after_gyms);
         #if SINGLE_SPECIES_RUN_SPARE_BITS > 0
            #if SINGLE_SPECIES_RUN_SPARE_BITS <= 8   
               lu_BitstreamWrite_u8(&state, 0, SINGLE_SPECIES_RUN_SPARE_BITS);
            #else
               lu_BitstreamWrite_u16(&state, 0, SINGLE_SPECIES_RUN_SPARE_BITS);
            #endif
         #endif
         break;
   }
   
   lu_BitstreamWrite_u8(&state, cgo.double_battles, 2);
   
   lu_BitstreamWrite_bool(&state, cgo.nuzlocke.enabled);
   //
   lu_BitstreamWrite_u8  (&state, cgo.nuzlocke.encounter_limit.type, 2);
   lu_BitstreamWrite_u8  (&state, cgo.nuzlocke.encounter_limit.gift_behavior, 2);
   lu_BitstreamWrite_u8  (&state, cgo.nuzlocke.encounter_limit.shiny_exception_base, 2);
   lu_BitstreamWrite_bool(&state, cgo.nuzlocke.encounter_limit.dupes_clause);
   lu_BitstreamWrite_u8  (&state, cgo.nuzlocke.encounter_limit.dupes_clause.shiny_exception, 2);
}

void BitReadCustomGameOptions(CustomGameOptions* cgo, lu_BitstreamState* state) {
   u16 version;
   
   version = lu_BitstreamRead_u16(&state, 16);
   if (version != CUSTOM_GAME_OPTION_DATA_VERSION) {
      // TODO: Fail; data corrupt.
      return;
   }
   
   cgo.allow_changing_these_during_play = lu_BitstreamRead_bool(&state);
   
   cgo.qol.start_with_running_shoes = lu_BitstreamRead_bool(&state);
   
   cgo.starter_mode = lu_BitstreamRead_bool(&state);
   switch (cgo.starter_mode) {
      case 0:
         cgo.starter_trio[0] = lu_BitstreamRead_u16(&state, BITS_FOR_POKEMON_SPECIES);
         cgo.starter_trio[1] = lu_BitstreamRead_u16(&state, BITS_FOR_POKEMON_SPECIES);
         cgo.starter_trio[2] = lu_BitstreamRead_u16(&state, BITS_FOR_POKEMON_SPECIES);
         break;
      case 1:
         cgo.single_species.player_species     = lu_BitstreamRead_u16(&state, BITS_FOR_POKEMON_SPECIES
         cgo.single_species.rival_base_species = lu_BitstreamRead_u16(&state, BITS_FOR_POKEMON_SPECIES);
         cgo.single_species.catch_mode = lu_BitstreamRead_u8(&state, 1);
         cgo.single_species.give_new_mon_after_gyms = lu_BitstreamRead_bool(&state);
         #if SINGLE_SPECIES_RUN_SPARE_BITS > 0
            #if SINGLE_SPECIES_RUN_SPARE_BITS <= 8   
               lu_BitstreamRead_u8(&state, SINGLE_SPECIES_RUN_SPARE_BITS);
            #else
               lu_BitstreamRead_u16(&state, SINGLE_SPECIES_RUN_SPARE_BITS);
            #endif
         #endif
         break;
   }
   
   cgo.double_battles = lu_BitstreamRead_u8(&state, 2);
   
   cgo.nuzlocke.enabled = lu_BitstreamRead_bool(&state);
   //
   cgo.nuzlocke.encounter_limit.type                 = lu_BitstreamRead_u8(&state, 2);
   cgo.nuzlocke.encounter_limit.gift_behavior        = lu_BitstreamRead_u8(&state, 2);
   cgo.nuzlocke.encounter_limit.shiny_exception_base = lu_BitstreamRead_u8(&state, 2);
   cgo.nuzlocke.encounter_limit.dupes_clause                 = lu_BitstreamRead_bool(&state);
   cgo.nuzlocke.encounter_limit.dupes_clause.shiny_exception = lu_BitstreamRead_u8  (&state, 2);
}

// 8/30/2023: 6 bytes total