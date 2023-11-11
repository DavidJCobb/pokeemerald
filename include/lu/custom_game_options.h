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

struct CustomGameScaleAndClamp {
   u16 scale; // percentage, i.e. 100 = 100%
   u16 min;
   u16 max;   // 0xFFFF = no max
};

u16 ApplyCustomGameScaleAndClamp_u16(u16, const struct CustomGameScaleAndClamp*);

u16 ApplyCustomGameScale_u16(u16, u16 scale);
u32 ApplyCustomGameScale_u32(u32, u16 scale);
s32 ApplyCustomGameScale_s32(s32, u16 scale);

#define UNIMPLEMENTED_CUSTOM_GAME_OPTION
#define UNTESTED_CUSTOM_GAME_OPTION

// Track current values of Custom Game options. Intended to be serialized after SaveBlock2.
extern struct CustomGameOptions {
#include "lu/generated/struct-members/CustomGameOptions.members.inl"
} gCustomGameOptions;

// Track in-game progress related to custom game options. Intended to be serialized after SaveBlock2.
extern struct CustomGameSavestate {
#include "lu/generated/struct-members/CustomGameSavestate.members.inl"
} gCustomGameSavestate;

#undef UNIMPLEMENTED_CUSTOM_GAME_OPTION
#undef UNTESTED_CUSTOM_GAME_OPTION

extern void ResetCustomGameOptions(void);
extern void ResetCustomGameSavestate(void);

extern void CustomGames_HandleNewPlaythroughStarted(void);

#endif