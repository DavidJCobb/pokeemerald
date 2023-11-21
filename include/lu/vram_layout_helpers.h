#ifndef GUARD_LU_VRAM_HELPERS
#define GUARD_LU_VRAM_HELPERS

#include "global.h"

typedef struct {
   u8 bytes[TILE_SIZE_4BPP];
} ALIGNED(TILE_SIZE_4BPP) VRAMTile;

typedef struct {
   u8 bytes[BG_SCREEN_SIZE / TILE_SIZE_4BPP];
} ALIGNED(BG_SCREEN_SIZE / TILE_SIZE_4BPP) VRAMTilemap;

#define VRAM_BG_MapBaseIndex(layout, member)   (offsetof((layout), (member)) / (BG_SCREEN_SIZE / TILE_SIZE_4BPP))

#define VRAM_BG_TileID(layout, member)   (offsetof((layout), (member)) / TILE_SIZE_4BPP)

#define VRAM_BG_CharBaseIndexToTileID(charBaseIndex)   (charBaseIndex * 512)

#define VRAM_BG_AT_CHAR_BASE_INDEX(charBaseIndex)   ALIGNED(sizeof(VRAMTile) * VRAM_BG_CharBaseIndexToTileID(charBaseIndex))

#define VRAM_BG_CharBasedTileID(charBaseIndex, layout, member)   ((offsetof((layout), (member)) / TILE_SIZE_4BPP) - VRAM_BG_CharBaseIndexToTileID(charBaseIndex))

/*

   Usage example:
   
      #define WIN_HEADER_TILE_WIDTH    DISPLAY_TILE_WIDTH
      #define WIN_HEADER_TILE_HEIGHT   2
      
      #define WIN_OPTIONS_TILE_WIDTH    DISPLAY_TILE_WIDTH
      #define WIN_OPTIONS_TILE_HEIGHT   14
      
      #define WIN_DIALOG_TILE_WIDTH    DISPLAY_TILE_WIDTH
      #define WIN_DIALOG_TILE_HEIGHT   DISPLAY_TILE_HEIGHT
      
      #define BG_LAYER_DIALOG_TILESET_INDEX   2
      
      enum {
         WIN_HEADER,
         WIN_OPTIONS,
         WIN_DIALOG,
      };
   
      // Never instantiated. Helper struct to control the VRAM layout for this menu.
      typedef struct {
         VRAMTile transparent_tile;
         VRAMTile blank_tile;
         VRAMTile user_window_frame[9];
         
         VRAMTilemap tilemaps[4];
         
         VRAMTile win_tiles_for_header[WIN_HEADER_TILE_WIDTH * WIN_HEADER_TILE_HEIGHT];
         VRAMTile win_tiles_for_options[WIN_OPTIONS_TILE_WIDTH * WIN_OPTIONS_TILE_HEIGHT];
         
         // We have a background layer set to use Tileset 2, able to address tiles in the 
         // range [1024, 2047].
         VRAMTile VRAM_BG_AT_CHAR_BASE_INDEX(BG_LAYER_DIALOG_TILESET_INDEX) blank_tile_for_help;
         VRAMTile win_tiles_for_dialog[WIN_DIALOG_TILE_WIDTH * WIN_DIALOG_TILE_HEIGHT];
      } VRAMTileLayout;
      
      STATIC_ASSERT(sizeof(VRAMTileLayout) <= BG_VRAM_SIZE, sStaticAssertion01_VramUsage);
      
      static const struct BgTemplate sOptionMenuBgTemplates[] = {
         {
            .bg = 0,
            //
            .charBaseIndex = 0,
            .mapBaseIndex  = VRAM_BG_MapBaseIndex(VRAMTileLayout, tilemaps[0]),
            .screenSize    = 0,
            .paletteMode   = 0,
            .priority      = 2,
            .baseTile      = 0
         },
         {
            .bg = 1,
            //
            .charBaseIndex = BG_LAYER_DIALOG_TILESET_INDEX,
            .mapBaseIndex  = VRAM_BG_MapBaseIndex(VRAMTileLayout, tilemaps[1]),
            .screenSize    = 0,
            .paletteMode   = 0,
            .priority      = 1,
            .baseTile      = 0
         },
      };
      
      static const struct WindowTemplate sOptionMenuWinTemplates[] = {
         [WIN_HEADER] = {
            .bg          = 0,
            .tilemapLeft = 0,
            .tilemapTop  = 0,
            .width       = WIN_HEADER_TILE_WIDTH,
            .height      = WIN_HEADER_TILE_HEIGHT,
            .paletteNum  = 0,
            .baseBlock   = VRAM_BG_TileID(VRAMTileLayout, win_tiles_for_header)
         },
         [WIN_OPTIONS] = {
            .bg          = 0,
            .tilemapLeft = 0,
            .tilemapTop  = 2,
            .width       = WIN_OPTIONS_TILE_WIDTH,
            .height      = WIN_OPTIONS_TILE_HEIGHT,
            .paletteNum  = 0,
            .baseBlock   = VRAM_BG_TileID(VRAMTileLayout, win_tiles_for_options)
         },
         [WIN_DIALOG] = {
            .bg          = 1,
            .tilemapLeft = 0,
            .tilemapTop  = 0,
            .width       = WIN_DIALOG_TILE_WIDTH,
            .height      = WIN_DIALOG_TILE_HEIGHT,
            .paletteNum  = 0,
            .baseBlock   = VRAM_BG_CharBasedTileID(BG_LAYER_DIALOG_TILESET_INDEX, VRAMTileLayout, win_tiles_for_dialog)
         },
      };
      
      // ...
      
      LoadUserWindowBorderGfxOnBg(0, VRAM_BG_TileID(VRAMTileLayout, user_window_frame), BG_PLTT_ID(7));

*/

#endif