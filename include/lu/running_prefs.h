#ifndef GUARD_LU_RUNNING_PREFS_H
#define GUARD_LU_RUNNING_PREFS_H

enum {
   LU_OVERWORLD_RUN_OPTION_VANILLA,
   LU_OVERWORLD_RUN_OPTION_TOGGLE, // "B" toggles running; "hold B" does nothing
};

extern u8   lu_GetOverworldRunOption(void);
extern void lu_SetOverworldRunOption(u8);

// When running is set to a toggle action rather than a hold action, these get and set 
// whether running is currently toggled.
extern bool8 lu_GetOverworldRunToggle(void);
extern bool8 lu_CanFlipOverworldRunToggle(void);
extern void  lu_FlipOverworldRunToggle(void);

extern bool8 lu_PlayerShouldRun(u16 heldKeys);

#endif