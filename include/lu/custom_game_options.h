#ifndef GUARD_LU_CUSTOM_GAME_OPTIONS
#define GUARD_LU_CUSTOM_GAME_OPTIONS
#include "lu/bitstreams.h"

enum {
   CustomGame_SingleSpeciesRun_CatchMode_OnlyCatchTargetSpecies,
   CustomGame_SingleSpeciesRun_CatchMode_PolymorphAllCaught,
};

enum {
   // Double Battles function as normal.
   CustomGame_DoubleBattles_Normal,
   
   // Double Battles function as normal, except that you can willingly enter a 1v2.
   CustomGame_DoubleBattles_Permissive,
   
   // All battles are Double Battles.
   CustomGame_DoubleBattles_Forced,
   
   // No battles are Double Battles.
   CustomGame_DoubleBattles_Disabled,
};

extern struct CustomGameOptions {
   // not yet implemented
   bool8 allow_changing_these_during_play;
   
   struct {
      // not yet implemented
      bool8 start_with_running_shoes;
   } qol;
   
   // not yet implemented
   bool8 starter_mode; // tag for next union
   union {
      u16 starter_trio[3]; // zero == use vanilla
      struct {
         u16   player_species;
         u16   rival_base_species;
         u8    catch_mode; // Only Catch Target Species / Polymorph Caught Pokemon into Target Species
         bool8 give_new_mon_after_gyms; // up to the 6th gym
      } single_species;
   } starter_data;
   
   // not yet implemented
   u8 double_battles;
   
   // not yet implemented
   struct {
      bool8 enabled;
      struct {
         u8 type;                  // Disabled / One per Location / First Encounter or Bust
         u8 gift_behavior;         // Free; Doesn't Count / Counts as Encounter / Disable all Gifts
         u8 shiny_exception_base;  // No Exception / Replace Prior from Same Route / Always Allow
         struct {
            bool8 enabled;         // Disabled / Enabled
            u8    shiny_exception; // No Exception / Replace Prior of Same Species / Always Allow
         } dupes_clause;
      } encounter_limit;
   } nuzlocke;
} gCustomGameOptions;

extern void ResetCustomGameOptions(void);

extern void BitWriteCustomGameOptions(struct CustomGameOptions*, struct lu_BitstreamState*);
extern void BitReadCustomGameOptions(struct CustomGameOptions*, struct lu_BitstreamState*);

#endif