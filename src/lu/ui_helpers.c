#include "lu/ui_helpers.h"

#include "bg.h"
#include "window.h"

void LuUI_DrawWindowFrame(
   u8  bg_layer,
   u16 dialog_frame_first_tile_id,
   u8  palette_id,
   u8  inner_x, // measured in tiles, not pixels
   u8  inner_y, // measured in tiles, not pixels
   u8  inner_w, // measured in tiles, not pixels
   u8  inner_h  // measured in tiles, not pixels
) {
   u8 l = inner_x - 1;
   u8 r = inner_x + inner_w;
   u8 t = inner_y - 1;
   u8 b = inner_y + inner_h;
   
   // C: corner
   // E: edge
   u8 tile_c_tl = dialog_frame_first_tile_id;
   u8 tile_e_t  = dialog_frame_first_tile_id + 1;
   u8 tile_c_tr = dialog_frame_first_tile_id + 2;
   u8 tile_e_l  = dialog_frame_first_tile_id + 3;
   u8 tile_mid  = dialog_frame_first_tile_id + 4;
   u8 tile_e_r  = dialog_frame_first_tile_id + 5;
   u8 tile_c_bl = dialog_frame_first_tile_id + 6;
   u8 tile_e_b  = dialog_frame_first_tile_id + 7;
   u8 tile_c_br = dialog_frame_first_tile_id + 8;
   
   FillBgTilemapBufferRect(bg_layer, tile_c_tl, l,     t,      1,        1,  palette_id);
   FillBgTilemapBufferRect(bg_layer, tile_e_t,  l + 1, t,      inner_w,  1,  palette_id);
   FillBgTilemapBufferRect(bg_layer, tile_c_tr, r,     t,      1,        1,  palette_id);
   FillBgTilemapBufferRect(bg_layer, tile_e_l,  l,     t + 1,  1,  inner_h,  palette_id);
   FillBgTilemapBufferRect(bg_layer, tile_e_r,  r,     t + 1,  1,  inner_h,  palette_id);
   FillBgTilemapBufferRect(bg_layer, tile_c_bl, l,     b,      1,        1,  palette_id);
   FillBgTilemapBufferRect(bg_layer, tile_e_b,  l + 1, b,      inner_w,  1,  palette_id);
   FillBgTilemapBufferRect(bg_layer, tile_c_br, r,     b,      1,        1,  palette_id);
}