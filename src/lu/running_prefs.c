#include "global.h"
#include "lu/running_prefs.h"

#include "global.fieldmap.h"
#include "field_player_avatar.h"
#include "constants/map_types.h" // MAP_TYPE_INDOOR
#include "event_data.h" // FlagGet

EWRAM_DATA static bool8 sRunToggleIsActive = FALSE;

u8 lu_GetOverworldRunOption(void) {
   return gSaveBlock2Ptr->optionsRunningToggle;
}
void lu_SetOverworldRunOption(u8 v) {
   gSaveBlock2Ptr->optionsRunningToggle = v;
   if (!v)
      sRunToggleIsActive = FALSE;
}

bool8 lu_GetOverworldRunToggle(void) {
   return sRunToggleIsActive;
}
bool8 lu_CanFlipOverworldRunToggle(void) {
   
   //
   // The following conditions are already checked for and excluded in `FieldGetPlayerInput`:
   //  - The player is undergoing a forced/scripted movement
   //
   
   // code duplicated from `bike.c` `IsRunningDisallowed(u8 metatile)`:
   if (!gMapHeader.allowRunning) {
      #if LU_ALLOW_RUNNING_INDOORS
         if (gMapHeader.mapType != MAP_TYPE_INDOOR)
            return TRUE;
         // else, if indoor, fall through to other checks.
         
         // to make a similar change for biking indoors, edit Overworld_IsBikingAllowed in overworld.c.
      #else
         return TRUE;
      #endif
   }
   
   if (TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_ACRO_BIKE | PLAYER_AVATAR_FLAG_MACH_BIKE | PLAYER_AVATAR_FLAG_SURFING | PLAYER_AVATAR_FLAG_UNDERWATER)) {
      return FALSE;
   }
   
   // Do not allow the player to toggle running before they have actually unlocked it as a feature:
   if (!FlagGet(FLAG_SYS_B_DASH)) {
      return FALSE;
   }
   
   return TRUE;
}
void lu_FlipOverworldRunToggle(void) {
   if (gSaveBlock2Ptr->optionsRunningToggle == LU_OVERWORLD_RUN_OPTION_VANILLA)
      return;
   sRunToggleIsActive = !sRunToggleIsActive;
}

bool8 lu_PlayerShouldRun(u16 heldKeys) {
   if (gSaveBlock2Ptr->optionsRunningToggle == LU_OVERWORLD_RUN_OPTION_VANILLA)
      return (heldKeys & B_BUTTON);
   return sRunToggleIsActive;
}