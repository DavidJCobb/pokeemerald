#include "global.h"
#include "lu/running_prefs.h"

#include "global.fieldmap.h"
#include "field_player_avatar.h"

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
   
   if (TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_ACRO_BIKE | PLAYER_AVATAR_FLAG_MACH_BIKE | PLAYER_AVATAR_FLAG_SURFING | PLAYER_AVATAR_FLAG_UNDERWATER)) {
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