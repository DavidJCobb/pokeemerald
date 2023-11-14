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

void LuUI_DrawBGScrollbarVert(
   u8 window_id,
   u8 pal_index_track,
   u8 pal_index_thumb,
   u8 pal_index_back,
   u8 x,          // pixels; X-offset of scrollbar left edge within window
   u8 width,      // pixels
   u8 inset_top,  // pixels; Y-offset of scrollbar top    edge within window
   u8 inset_btm,  // pixels; Y-offset of scrollbar bottom edge within window
   u8 scroll_pos, // items
   u8 item_count, // items
   u8 max_visible_items // max number of items visible at a time, in the list pane
) {
   u8* const win_tiles = gWindows[window_id].tileData;
   //
   u8 track_height  = (gWindows[window_id].window.height * TILE_HEIGHT) - inset_top - inset_btm;
   u8 thumb_pos     = track_height * scroll_pos / item_count;
   u8 thumb_size    = track_height * max_visible_items / item_count;
   u8 tiles_per_row = gWindows[window_id].window.width;
   
   #ifndef NDEBUG
   {
      u16 inset_sum  = (u16)inset_top + inset_bottom;
      u16 win_height = gWindows[window_id].window.height * TILE_HEIGHT;
      u16 right_edge = (u16)x + width;
      
      AGB_ASSERT(inset_sum  < win_height); // assert: scrollbar height > 0
      AGB_ASSERT(right_edge < tiles_per_row * TILE_WIDTH); // bounds-check X against window width
   }
   #endif
   
   if (thumb_size == 0) {
      thumb_size = 1;
   }
   
   // thumb_pos is relative to the start of the scrollbar track, not screen-relative
   
   if (item_count <= max_visible_items) {
      pal_index_track = pal_index_back;
      pal_index_thumb = pal_index_back;
   }
   {
      u32 cur_x; // pixels
      u32 cur_y; // pixels
      for(cur_y = 0; cur_y < track_height; ++cur_y) {
         u8 tile_y  = (cur_y + inset_top) / TILE_HEIGHT;
         u8 pixel_y = (cur_y + inset_top) % TILE_HEIGHT;
         for(cur_x = x; cur_x < x + width; ++cur_x) {
            u8 tile_x  = cur_x / TILE_WIDTH;
            u8 pixel_x = cur_x % TILE_WIDTH;
            //
            // Each 8x8px tile uses four bits per pixel and so consumes 0x20 bytes. 
            // Pixels are arranged row-major.
            //
            u16 tile_index = (tile_y * tiles_per_row) + tile_x;
            u8* dst_tile   = &win_tiles[tile_index * 0x20];
            u8* dst_byte   = &dst_tile[(pixel_y * 4) + (pixel_x / 2)];
            //
            u8 pal_index = pal_index_track;
            if (cur_y >= thumb_pos && cur_y <= thumb_pos + thumb_size) {
               pal_index = pal_index_thumb;
            }
            if (!(pixel_x & 1)) { // LSBs = left; MSBs = right
               *dst_byte &= 0xF0;
               *dst_byte |= pal_index;
            } else {
               *dst_byte &= 0x0F;
               *dst_byte |= (pal_index << 4);
            }
         }
      }
   }
}