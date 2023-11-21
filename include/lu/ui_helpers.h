#ifndef GUARD_LU_UI_HELPERS
#define GUARD_LU_UI_HELPERS

//
// Helper functions for UIs.
//
// TIP: You should also consult `include/menu_helpers.h`.
//
// TIP: You should also consult `include/menu.h` for:
//       - AddTextPrinterForMessage
//       - AddTextPrinterParameterized2 and other overloads
//       - GetPlayerTextSpeed
//       - GetPlayerTextSpeedDelay
//       - GetStartMenuWindowId
//

#include "global.h"

// "Reset" functions, usable when opening a new menu. Broken up in order to allow you 
// to avoid doing too much work in one frame; Game Freak takes the same approach.
void LuUI_ResetBackgroundsAndVRAM(void);
void LuUI_ResetSpritesAndEffects(void);

// Load the palette and tiles for the player's preferred text window frame border.
void LuUI_LoadPlayerWindowFrame(u8 bg_layer, u8 bg_palette_index, u16 destination_tile_id);

// =========================================================================================

// Prior to calling this, your menu should've called:
//    LuUI_LoadPlayerWindowFrame(bg_layer, palette_id, dialog_frame_first_tile_id);
//
// This function is designed such that if you treat a Window as a content-box 
// and pass its coordinates/size verbatim, this will draw a border-box for it. 
// It only draws the frame border; it doesn't modify the contents of the window 
// itself; and it doesn't apply any padding to the coordinates: the border box is 
// flush with the edge of the content box.
//
// Game Freak's `menu.h` offers `DrawDialogFrameWithCustomTileAndPalette`, which is 
// similar except that it takes a window ID rather than coordinates, and it adds an 
// extra tile of padding around that window's content box. It also fills the window 
// to blank before drawing the border; it's designed to be the first function you'd 
// call when drawing a dialog box.
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


// These functions use GBA I/O screen windows to darken a specific background 
// layer except for a given rect. The Options Window uses this effect to dim 
// the options list and highlight the current option.
//
// This relies on screen window 0, assumes screen window 1 is not enabled, and 
// sets the screen color effect.
void LuUI_SetupDarkenAllExceptRect(
   u8 bg_layer_to_darken
);
void LuUI_DoDarkenAllExceptRect(
   u8 x,     // pixels
   u8 y,     // pixels
   u8 width, // pixels
   u8 height // pixels
);
void LuUI_StopDarkenAllExceptRect(
   u8 bg_layer_to_darken
);

#endif