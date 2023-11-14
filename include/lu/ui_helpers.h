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

// Draws a simple scrollbar onto the background layer. Assumes 4-bit color depth. 
// Does not bounds-check.
void LuUI_DrawBGScrollbarVert(
   u8 window_id,
   u8 pal_index_track,
   u8 pal_index_thumb,
   u8 pal_index_back,
   u8 x,          // pixels
   u8 width,      // pixels
   u8 inset_top,  // pixels
   u8 inset_btm,  // pixels
   u8 scroll_pos, // items
   u8 item_count, // items
   u8 max_visible_items // max number of items visible at a time, in the list pane
);

#endif