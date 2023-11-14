#ifndef GUARD_LU_UI_HELPERS
#define GUARD_LU_UI_HELPERS

#include "global.h"

// Prior to calling this, your menu should've called:
//    LoadUserWindowBorderGfx(bg_layer, dialog_frame_first_tile_id, BG_PLTT_ID(palette_id));
//    LoadPalette(GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->pal, BG_PLTT_ID(palette_id), PLTT_SIZE_4BPP);
//
// This function is designed such that if you treat a Window as a content-box 
// and pass its coordinates/size verbatim, this will draw a border-box for it.
//
void LuUI_DrawWindowFrame(
   u8  bg_layer,
   u16 dialog_frame_first_tile_id,
   u8  palette_id,
   u8  inner_x, // measured in tiles, not pixels
   u8  inner_y, // measured in tiles, not pixels
   u8  inner_w, // measured in tiles, not pixels
   u8  inner_h  // measured in tiles, not pixels
);

#endif