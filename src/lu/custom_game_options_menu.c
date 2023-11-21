#include "global.h"
#include "lu/custom_game_options_menu.h"
#include "lu/custom_game_options.h"

#include "bg.h"
#include "decompress.h" // LoadCompressedSpriteSheet
#include "gpu_regs.h"
#include "main.h"
#include "malloc.h" // AllocZeroed, Free
#include "menu.h"
#include "palette.h"
#include "scanline_effect.h"
#include "sound.h" // PlaySE
#include "sprite.h"
#include "strings.h" // literally just for the "Cancel" option lol
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "gba/m4a_internal.h"
#include "constants/rgb.h"
#include "constants/songs.h" // SE_SELECT and other sound effect constants

#include "graphics.h" // interface sprites

#include "characters.h"
#include "string_util.h"
#include "lu/string_wrap.h"
#include "lu/strings.h"
#include "lu/ui_helpers.h"

/*//

   The current menu design is as follows:
   
    - Most of the GUI is on a shared background layer.
    
    - The list of options is on its own background layer. Screen color effects and 
      blending are used to darken all options except the row the player has their 
      cursor on. This is the same basic effect used in the vanilla Options menu. 
      It's set up in `SetUpHighlightEffect`, and the non-darkened area is positioned 
      in `HighlightCGOptionMenuItem`.
      
       - The scrollbar paints to this layer, too.
      
    - The help text is on its own background layer as well, set to display overtop 
      all others. The help text window is sized so that it doesn't cover the menu 
      header or keybind strip, so those always show through.
      
    - Because the screen color effects used by `SetUpHighlightEffect` are set up to 
      only darken the layer used for the options list, we don't actually have to 
      disable the effect when showing help text. It gets covered up too.
   
   
   
   Potential design improvements:
   
    - It'd be nice if pressing A on an integral option would open a small editor 
      allowing you to type in a new numeric value. We could display the editor 
      directly overtop the option value, with a 3x4 grid of digits to enter (0-9) 
      and OK/Cancel buttons you can navigate to. We'd have to implement a usable 
      cursor, among other things.
      
      I feel like this might be a useful UI to implement as a general-purpose 
      thing: the ability to call a function and say, "Hey, buddy, spawn this 
      widget at W screen offset on X background layer, and use Y task ID until 
      you're done; then run Z callback." We could put this widget on the Help 
      layer.
      
       - Being able to press A on enum options to get a drop-down would be nice, 
         too.
   
    - Submenus should have some kind of icon like ">>" shown where a value would 
      be.
      
    - We should find an alternate way to indicate the current menu item, as the 
      dimming effect currently dims the scrollbar, too.
      
    - We need theming in general but especially for the Help page.
      
    - It'd be nice if the Help text were scrollable, so we wouldn't have to 
      worry about it being too long to display. Maybe we could enable Window 1 
      and use it for the help screen: set the help background layer to a size of 
      256x512px, and only enable it within LCD Window 1, not LCD Window 0 or the 
      Not-in-Window region; then, we can modify the layer's position to scroll it, 
      with LCD Window 1 cropping it.
   
    - An enum option can be defined to have help text for the option as a whole, 
      and for each individual value. However, we only display one. It'd be nice 
      if we could draw both, though maybe that should require a MENUITEM_FLAG_.
      
       - Let's hold off on this until we actually have enough enum options to 
         see if it'd be terribly helpful.
   
    - We need a separate UI for rendering Pokemon species selection, with the 
      ability to search by type and by name substring. This is required in order 
      to implement options that let you pick a species, like overriding the game's 
      starter Pokemon, or forcing single-species runs.
      
       - The Pokedex has filter-and-sort functionality, though the actual sort 
         and filter criteria available are very limited. It'd be useful to see 
         just how they implemented generating the list of species to display. 
         Their actual search form, however, isn't worth copying; we should design 
         our own.
         
          - Since they use a heap-allocated struct for the Pokedex UI state, they 
            literally just have an array of dex list items (dex number, seen flag, 
            and owned flag) as a member, with extent `NATIONAL_DEX_COUNT + 1`. 
            Guess we'll do the same, then.

//*/

enum {
   WIN_HEADER,
   WIN_KEYBINDS_STRIP,
   WIN_OPTIONS,
   WIN_HELP,
   //
   WIN_COUNT
};

#define SOUND_EFFECT_MENU_SCROLL   SE_SELECT
#define SOUND_EFFECT_VALUE_SCROLL  SE_SELECT
#define SOUND_EFFECT_SUBMENU_ENTER SE_SELECT
#define SOUND_EFFECT_SUBMENU_EXIT  SE_SELECT
#define SOUND_EFFECT_HELP_EXIT     SE_SELECT

EWRAM_DATA static struct CustomGameOptions sTempOptions;

#include "lu/custom_game_options_menu/menu_item.h"
#include "lu/custom_game_options_menu/menu_hierarchy_definition.h" // sTopLevelMenu is the top-level menu


//
// Menu state and associated funcs:
//

#define MAX_MENU_TRAVERSAL_DEPTH 8

struct MenuStackFrame {
   const u8* name;
   const struct CGOptionMenuItem* menu_items;
};

struct MenuState {
   struct MenuStackFrame breadcrumbs[MAX_MENU_TRAVERSAL_DEPTH];
   u8    cursor_pos;
   bool8 is_in_help;
   u8    sprite_id_value_arrow_l;
   u8    sprite_id_value_arrow_r;
};

static EWRAM_DATA struct MenuState* sMenuState = NULL;

static void ResetMenuState(void);

static void UpdateDisplayedMenuName(void);
static void UpdateDisplayedMenuItems(void);
static u8 GetScreenRowForCursorPos(void);

static const struct CGOptionMenuItem* GetCurrentMenuItemList(void);

static void EnterSubmenu(const u8* submenu_name, const struct CGOptionMenuItem* submenu_items);
static bool8 TryExitSubmenu();

static void TryMoveMenuCursor(s8 by);

//
// Menu state and associated funcs:
//

static void ResetMenuState(void) {
   u8 i;
   
   sMenuState->breadcrumbs[0].name       = gText_lu_CGO_menuTitle;
   sMenuState->breadcrumbs[0].menu_items = sTopLevelMenu;
   for(i = 1; i < MAX_MENU_TRAVERSAL_DEPTH; ++i) {
      sMenuState->breadcrumbs[i].name       = NULL;
      sMenuState->breadcrumbs[i].menu_items = NULL;
   }
   sMenuState->cursor_pos = 0;
   sMenuState->is_in_help = FALSE;
}

static const struct CGOptionMenuItem* GetCurrentMenuItemList(void) {
   u8 i;
   const struct CGOptionMenuItem* items;
   
   items = sTopLevelMenu;
   for(i = 0; i < MAX_MENU_TRAVERSAL_DEPTH; ++i) {
      if (sMenuState->breadcrumbs[i].menu_items != NULL) {
         items = sMenuState->breadcrumbs[i].menu_items;
      } else {
         break;
      }
   }
   return items;
}

static void EnterSubmenu(const u8* submenu_name, const struct CGOptionMenuItem* submenu_items) {
   u8    i;
   bool8 success;
   
   success = FALSE;
   for(i = 0; i < MAX_MENU_TRAVERSAL_DEPTH; ++i) {
      if (sMenuState->breadcrumbs[i].menu_items == NULL) {
         sMenuState->breadcrumbs[i].name       = submenu_name;
         sMenuState->breadcrumbs[i].menu_items = submenu_items;
         success = TRUE;
         break;
      }
   }
   if (!success) {
      // TODO: DebugPrint
      return;
   }
   
   sMenuState->cursor_pos = 0;
   
   PlaySE(SOUND_EFFECT_SUBMENU_ENTER);
   UpdateDisplayedMenuName();
   UpdateDisplayedMenuItems();
}
static bool8 TryExitSubmenu() { // returns FALSE if at top-level menu
   u8    i;
   bool8 success;
   
   success = FALSE;
   for(i = MAX_MENU_TRAVERSAL_DEPTH - 1; i > 0; --i) {
      if (sMenuState->breadcrumbs[i].menu_items != NULL) {
         sMenuState->breadcrumbs[i].name       = NULL;
         sMenuState->breadcrumbs[i].menu_items = NULL;
         success = TRUE;
         break;
      }
   }
   if (!success) {
      return FALSE; // in top-level menu
   }
   
   sMenuState->cursor_pos = 0; // TODO: use the scroll offset of the last entered submenu
   
   PlaySE(SOUND_EFFECT_SUBMENU_EXIT);
   UpdateDisplayedMenuName();
   UpdateDisplayedMenuItems();
   
   return TRUE;
}

static void TryMoveMenuCursor(s8 by) {
   u8 items_count = GetMenuItemListCount(GetCurrentMenuItemList());
   
   if (by < 0) {
      if (sMenuState->cursor_pos < -by) {
         sMenuState->cursor_pos = items_count - 1;
      } else {
         sMenuState->cursor_pos += by;
      }
   }
   if (by > 0) {
      sMenuState->cursor_pos += by;
      if (sMenuState->cursor_pos >= items_count) {
         sMenuState->cursor_pos = 0;
      }
   }
   PlaySE(SOUND_EFFECT_MENU_SCROLL);
}

//
// Sprites:
//

#define SPRITE_TAG_VALUE_ARROW 4096

static void SpriteCB_ValueArrow(struct Sprite*);

static const struct OamData sOamData_ValueArrow = {
    .y           = 0,
    .affineMode  = ST_OAM_AFFINE_OFF,
    .objMode     = ST_OAM_OBJ_NORMAL,
    .mosaic      = FALSE,
    .bpp         = ST_OAM_4BPP,
    .shape       = SPRITE_SHAPE(8x8),
    .x           = 0,
    .matrixNum   = 0,
    .size        = SPRITE_SIZE(8x8),
    .tileNum     = 0,
    .priority    = 1,
    .paletteNum  = 0,
    .affineParam = 0
};

static const union AnimCmd sSpriteAnim_ValueArrow[] = {
   ANIMCMD_FRAME(0, 0),
   ANIMCMD_END
};
//
static const union AnimCmd* const sSpriteAnimTable_ValueArrow[] = {
   sSpriteAnim_ValueArrow
};

static const struct SpriteTemplate sSpriteTemplate_ValueArrow = {
    .tileTag     = SPRITE_TAG_VALUE_ARROW,
    .paletteTag  = SPRITE_TAG_VALUE_ARROW,
    .oam         = &sOamData_ValueArrow,
    .anims       = sSpriteAnimTable_ValueArrow,
    .images      = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback    = SpriteCB_ValueArrow,
};

static const u32 sInterfaceSpriteGfx[] = INCBIN_U32("graphics/lu/cgo_menu/interface-sprites.4bpp.lz");
static const u16 sInterfaceSpritePal[] = INCBIN_U16("graphics/lu/cgo_menu/interface-sprites.gbapal");

static const struct CompressedSpriteSheet sInterfaceSpriteSheet[] = {
   {sInterfaceSpriteGfx, 0x2000, SPRITE_TAG_VALUE_ARROW},
   {0}
};
static const struct SpritePalette sInterfaceSpritePalette[] = {
   {sInterfaceSpritePal, SPRITE_TAG_VALUE_ARROW},
   {0}
};

static void CreateInterfaceSprites(void);
static void PositionValueArrowsAtRow(u8 screen_row);

//
// Task and drawing stuff:
//

static void Task_CGOptionMenuFadeIn(u8 taskId);
static void Task_CGOptionMenuProcessInput(u8 taskId);
static void Task_CGOptionMenuSave(u8 taskId);
static void Task_CGOptionMenuFadeOut(u8 taskId);
static void HighlightCGOptionMenuItem();

static void SpriteCB_ScrollbarThumb(struct Sprite*);

static void TryDisplayHelp(const struct CGOptionMenuItem* item);

static void UpdateDisplayedMenuName(void);
static void DrawMenuItem(const struct CGOptionMenuItem* item, u8 row, bool8 is_single_update);
static void UpdateDisplayedMenuItems(void);
static void RepaintScrollbar(void);
static void UpdateDisplayedControls(void);

// note: this is only used in the Japanese release
static const u8 sMenuCursorBGTiles[] = INCBIN_U8("graphics/lu/cgo_menu/bg-tiles-cursor.4bpp");
static const u8 sBlankBGTile[] = INCBIN_U8("graphics/lu/cgo_menu/bg-tile-blank.4bpp"); // color 1

static const u16 sOptionsListingPalette[] = INCBIN_U16("graphics/lu/cgo_menu/option_listing.gbapal");
//
// Background, foreground, shadow:
//
static const u8 sTextColor_OptionNames[] = {1, 2, 3};
static const u8 sTextColor_OptionValues[] = {1, 4, 5};
static const u8 sTextColor_HelpBodyText[] = {1, 2, 3};

// BG palette for options window area:
//  0 transparent
//  1 background color (white)
//  2 option name text
//  3 option name shadow
//  4 option value text
//  5 option value shadow
//  6 ?
//  7 ?
//  8 ?
//  9 ?
// 14 scrollbar track
// 15 scrollbar thumb
#define SCROLLBAR_PALETTE_INDEX_BLANK 1
#define SCROLLBAR_PALETTE_INDEX_TRACK 14
#define SCROLLBAR_PALETTE_INDEX_THUMB 15

#define BACKGROUND_LAYER_NORMAL  0
#define BACKGROUND_LAYER_OPTIONS 1
#define BACKGROUND_LAYER_HELP    2

#define BACKGROUND_PALETTE_ID_MENU     0
#define BACKGROUND_PALETTE_ID_TEXT     1
#define BACKGROUND_PALETTE_ID_CONTROLS 2
#define BACKGROUND_PALETTE_BOX_FRAME   7

#define TEXT_ROW_HEIGHT_IN_TILES   2

#define WIN_HEADER_TILE_WIDTH    DISPLAY_TILE_WIDTH
#define WIN_HEADER_TILE_HEIGHT   TEXT_ROW_HEIGHT_IN_TILES
//
#define WIN_KEYBIND_STRIP_TILE_WIDTH    DISPLAY_TILE_WIDTH
#define WIN_KEYBIND_STRIP_TILE_HEIGHT   TEXT_ROW_HEIGHT_IN_TILES
//
#define WIN_OPTIONS_X_TILES             3
#define WIN_OPTIONS_Y_TILES             WIN_HEADER_TILE_HEIGHT + 1
#define OPTIONS_INSET_RIGHT_TILES       2
#define OPTIONS_LIST_INSET_RIGHT        (OPTIONS_INSET_RIGHT_TILES * TILE_WIDTH)
#define OPTIONS_LIST_ROW_HEIGHT         7
//
#define WIN_OPTIONS_TILE_WIDTH    (DISPLAY_TILE_WIDTH - WIN_OPTIONS_X_TILES)
#define WIN_OPTIONS_TILE_HEIGHT   (OPTIONS_LIST_ROW_HEIGHT * TEXT_ROW_HEIGHT_IN_TILES)
//
#define WIN_HELP_TILE_WIDTH    DISPLAY_TILE_WIDTH
#define WIN_HELP_TILE_HEIGHT   (DISPLAY_TILE_HEIGHT - WIN_HEADER_TILE_HEIGHT - WIN_KEYBIND_STRIP_TILE_HEIGHT)
//
#define MAX_MENU_ITEMS_VISIBLE_AT_ONCE   OPTIONS_LIST_ROW_HEIGHT
#define MENU_ITEM_HALFWAY_ROW            (MAX_MENU_ITEMS_VISIBLE_AT_ONCE / 2)

#define OPTION_VALUE_COLUMN_WIDTH  80
#define OPTION_VALUE_COLUMN_X      (DISPLAY_WIDTH - OPTIONS_LIST_INSET_RIGHT - OPTION_VALUE_COLUMN_WIDTH)

#define SCROLLBAR_X     230
#define SCROLLBAR_WIDTH   3
#define SCROLLBAR_TRACK_HEIGHT   (WIN_OPTIONS_TILE_HEIGHT * TILE_HEIGHT)



#define BACKGROUND_LAYER_NORMAL_TILESET    0 // Start from Tile ID 0.
#define BACKGROUND_LAYER_OPTIONS_TILESET   0 // Start from Tile ID 0.
#define BACKGROUND_LAYER_HELP_TILESET      2 // Start from Tile ID 512.

// Never instantiated. Just a marginally less hideous way to manage all this 
// compared to preprocessor macros. Unit of measurement is 4bpp tile IDs.
typedef struct {
   u8 transparent_tile;
   u8 blank_tile;
   u8 selection_cursor_tiles[2];
   u8 user_window_frame[9];
   
   u8 ALIGNED(BG_SCREEN_SIZE / TILE_SIZE_4BPP) tilemaps[4][BG_SCREEN_SIZE / TILE_SIZE_4BPP];
   
   u8 win_tiles_for_header[WIN_HEADER_TILE_WIDTH * WIN_HEADER_TILE_HEIGHT];
   u8 win_tiles_for_keybinds[WIN_KEYBIND_STRIP_TILE_WIDTH * WIN_KEYBIND_STRIP_TILE_HEIGHT];
   u8 win_tiles_for_options[WIN_OPTIONS_TILE_WIDTH * WIN_OPTIONS_TILE_HEIGHT];
   
   // We have the help screen set to use Tileset 2, so it can address tiles in the range [1536, 2047].
   // We want to skip a tile so that we have a blank/transparent tile we can display.
   u8 ALIGNED(1024) blank_tile_for_help;
   u8 win_tiles_for_help[WIN_HELP_TILE_WIDTH * WIN_HELP_TILE_HEIGHT];
} VRamTileLayout;

// ensure we fit within 64KB VRAM limit.
STATIC_ASSERT(sizeof(VRamTileLayout) <= (BG_VRAM_SIZE / TILE_SIZE_4BPP), sCGOMenuStaticAssertion01_VramUsage);

#define DIALOG_FRAME_FIRST_TILE_ID    offsetof(VRamTileLayout, user_window_frame)

static const struct BgTemplate sOptionMenuBgTemplates[] = {
   {
      .bg = BACKGROUND_LAYER_NORMAL,
      //
      .charBaseIndex = BACKGROUND_LAYER_NORMAL_TILESET,
      .mapBaseIndex  = offsetof(VRamTileLayout, tilemaps[BACKGROUND_LAYER_NORMAL]) / 64,
      .screenSize    = 0,
      .paletteMode   = 0,
      .priority      = 2,
      .baseTile      = 0
   },
   {
      .bg = BACKGROUND_LAYER_OPTIONS,
      //
      .charBaseIndex = BACKGROUND_LAYER_OPTIONS_TILESET,
      .mapBaseIndex  = offsetof(VRamTileLayout, tilemaps[BACKGROUND_LAYER_OPTIONS]) / 64,
      .screenSize    = 0,
      .paletteMode   = 0,
      .priority      = 1,
      .baseTile      = 0
   },
   {
      .bg = BACKGROUND_LAYER_HELP,
      //
      .charBaseIndex = BACKGROUND_LAYER_HELP_TILESET,
      .mapBaseIndex  = offsetof(VRamTileLayout, tilemaps[BACKGROUND_LAYER_HELP]) / 64,
      .screenSize    = 0,
      .paletteMode   = 0,
      .priority      = 0,
      .baseTile      = 0
   },
};

static const struct WindowTemplate sOptionMenuWinTemplates[] = {
    [WIN_HEADER] = {
        .bg          = BACKGROUND_LAYER_NORMAL,
        .tilemapLeft = 0,
        .tilemapTop  = 0,
        .width       = WIN_HEADER_TILE_WIDTH,
        .height      = WIN_HEADER_TILE_HEIGHT,
        .paletteNum  = BACKGROUND_PALETTE_ID_CONTROLS,
        .baseBlock   = offsetof(VRamTileLayout, win_tiles_for_header) - (BACKGROUND_LAYER_NORMAL_TILESET * 512)
    },
    [WIN_KEYBINDS_STRIP] = {
        .bg          = BACKGROUND_LAYER_NORMAL,
        .tilemapLeft = 0,
        .tilemapTop  = DISPLAY_TILE_HEIGHT - 2,
        .width       = WIN_KEYBIND_STRIP_TILE_WIDTH,
        .height      = WIN_KEYBIND_STRIP_TILE_HEIGHT,
        .paletteNum  = BACKGROUND_PALETTE_ID_CONTROLS,
        .baseBlock   = offsetof(VRamTileLayout, win_tiles_for_keybinds) - (BACKGROUND_LAYER_NORMAL_TILESET * 512)
    },
    [WIN_OPTIONS] = {
        .bg          = BACKGROUND_LAYER_OPTIONS,
        .tilemapLeft = WIN_OPTIONS_X_TILES,
        .tilemapTop  = WIN_OPTIONS_Y_TILES,
        .width       = WIN_OPTIONS_TILE_WIDTH,
        .height      = WIN_OPTIONS_TILE_HEIGHT,
        .paletteNum  = BACKGROUND_PALETTE_ID_TEXT,
        .baseBlock   = offsetof(VRamTileLayout, win_tiles_for_options) - (BACKGROUND_LAYER_OPTIONS_TILESET * 512)
    },
    [WIN_HELP] = {
        .bg          = BACKGROUND_LAYER_HELP,
        .tilemapLeft = 0,
        .tilemapTop  = 2,
        .width       = WIN_HELP_TILE_WIDTH,
        .height      = WIN_HELP_TILE_HEIGHT,
        .paletteNum  = BACKGROUND_PALETTE_ID_TEXT,
        .baseBlock   = offsetof(VRamTileLayout, win_tiles_for_help) - (BACKGROUND_LAYER_HELP_TILESET * 512)
    },
    //
    [WIN_COUNT] = DUMMY_WIN_TEMPLATE
};

static const u16 sOptionMenuBg_Pal[] = {RGB(17, 18, 31)};

static void MainCB2(void) {
   RunTasks();
   AnimateSprites();
   BuildOamBuffer();
   UpdatePaletteFade();
}
static void VBlankCB(void) {
   LoadOam();
   ProcessSpriteCopyRequests();
   TransferPlttBuffer();
}

static void SetUpHighlightEffect(void) {
   //
   // Set up the visual effect we'll be using to highlight the selected option.
   //
   // We set the current GBA color effect to "darken," and enable it for the options 
   // layer. We set up LCD Window 0 (not to be confused with UI windows) to show the 
   // options layer, and set the "Not In Any LCD Window" region to show all background 
   // layers and the color effect. We'll use LCD Window 0 as a cutout for the darken 
   // effect: in the options window, any pixel that falls inside of Window 0 won't be 
   // darkened.
   //
   
   /*//
   LuUI_SetupDarkenAllExceptRect(BACKGROUND_LAYER_OPTIONS);
   //*/
}

void CB2_InitCustomGameOptionMenu(void) {
   switch (gMain.state) {
      default:
      case 0:
         SetVBlankCallback(NULL);
         gMain.state++;
         break;
      case 1:
         LuUI_ResetBackgroundsAndVRAM();
         InitBgsFromTemplates(0, sOptionMenuBgTemplates, ARRAY_COUNT(sOptionMenuBgTemplates));
           
         InitWindows(sOptionMenuWinTemplates);
         DeactivateAllTextPrinters();
         
         SetUpHighlightEffect();
         
         ShowBg(BACKGROUND_LAYER_NORMAL);
         ShowBg(BACKGROUND_LAYER_OPTIONS);
         //HideBg(BACKGROUND_LAYER_HELP); // done by `LuUI_ResetBackgroundsAndVRAM`
         
         // Enable sprite layer:
         SetGpuReg(REG_OFFSET_DISPCNT, GetGpuReg(REG_OFFSET_DISPCNT) | DISPCNT_OBJ_ON);
         
         gMain.state++;
         break;
       case 2:
         //
         // Reset visual effect states and sprites, and forcibly terminate all running tasks. 
         // Running tasks won't be given an opportunity to run any teardown or cleanup code, 
         // e.g. freeing any memory allocations they've made, so it's important that the 
         // Custom Game Options window only be run under controlled conditions. (The Options 
         // window and some other Game Freak UI does this, too.)
         //
         LuUI_ResetSpritesAndEffects();
         ResetTasks();
         LoadCompressedSpriteSheet(&sInterfaceSpriteSheet[0]);
         LoadSpritePalettes(sInterfaceSpritePalette);
         {
            LoadBgTiles(BACKGROUND_LAYER_OPTIONS, sBlankBGTile, TILE_SIZE_4BPP, offsetof(VRamTileLayout, blank_tile));
            LoadBgTiles(BACKGROUND_LAYER_OPTIONS, sMenuCursorBGTiles, TILE_SIZE_4BPP * 2, offsetof(VRamTileLayout, selection_cursor_tiles));
         }
         gMain.state++;
         break;
       case 3:
         LuUI_LoadPlayerWindowFrame(
            BACKGROUND_LAYER_OPTIONS,
            BACKGROUND_PALETTE_BOX_FRAME,
            DIALOG_FRAME_FIRST_TILE_ID
         );
         gMain.state++;
         break;
       case 4:
         LoadPalette(sOptionMenuBg_Pal, BG_PLTT_ID(BACKGROUND_PALETTE_ID_MENU), sizeof(sOptionMenuBg_Pal));
         gMain.state++;
         break;
       case 5:
         LoadPalette(sOptionsListingPalette, BG_PLTT_ID(BACKGROUND_PALETTE_ID_TEXT), sizeof(sOptionsListingPalette));
         LoadPalette(GetTextWindowPalette(2), BG_PLTT_ID(BACKGROUND_PALETTE_ID_CONTROLS), PLTT_SIZE_4BPP);
         gMain.state++;
         break;
       case 6:
         PutWindowTilemap(WIN_HEADER);
         gMain.state++;
         break;
       case 7:
         PutWindowTilemap(WIN_HELP);
         gMain.state++;
         break;
       case 8:
         PutWindowTilemap(WIN_OPTIONS);
         FillWindowPixelBuffer(WIN_OPTIONS, PIXEL_FILL(1));
         CopyWindowToVram(WIN_OPTIONS, COPYWIN_FULL);
         gMain.state++;
         break;
       case 9:
         PutWindowTilemap(WIN_KEYBINDS_STRIP);
         gMain.state++;
         break;
       case 10:
         {
            u8 taskId = CreateTask(Task_CGOptionMenuFadeIn, 0);
            {
               sMenuState = AllocZeroed(sizeof(struct MenuState));
               ResetMenuState();
               CreateInterfaceSprites();
            }
            sTempOptions = gCustomGameOptions;
            UpdateDisplayedMenuName();
            UpdateDisplayedMenuItems();
            UpdateDisplayedControls();
            HighlightCGOptionMenuItem();

            CopyWindowToVram(WIN_OPTIONS, COPYWIN_FULL);
            gMain.state++;
         }
         break;
       case 11:
         BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
         SetVBlankCallback(VBlankCB);
         SetMainCallback2(MainCB2);
         return;
   }
}


#define sDirectionX data[0]
#define sMoving     data[1]

// Ideally 5, as that's the default `gKeyRepeatContinueDelay`.
#define VALUE_ARROW_MOVE_DISTANCE 5

static void CreateInterfaceSprites(void) {
   u8 id_l;
   u8 id_r;
   u8 x;
   
   sMenuState->sprite_id_value_arrow_l = id_l = CreateSprite(&sSpriteTemplate_ValueArrow, 0, 0, 0);
   sMenuState->sprite_id_value_arrow_r = id_r = CreateSprite(&sSpriteTemplate_ValueArrow, 0, 0, 0);
   
   gSprites[id_r].hFlip = TRUE;
   
   gSprites[id_l].sDirectionX = -1;
   gSprites[id_r].sDirectionX =  1;
   //
   gSprites[id_l].sMoving = 0;
   gSprites[id_r].sMoving = 0;
   
   x = (sOptionMenuWinTemplates[WIN_OPTIONS].tilemapLeft * TILE_WIDTH);
   
   gSprites[id_l].x = x + OPTION_VALUE_COLUMN_X;
   gSprites[id_r].x = x + OPTION_VALUE_COLUMN_X + OPTION_VALUE_COLUMN_WIDTH;
}
static void PositionValueArrowsAtRow(u8 screen_row) {
   u8 y = (WIN_OPTIONS_Y_TILES * TILE_HEIGHT) + (screen_row * 16) + 6 + (TILE_HEIGHT * 2);
   // 6 == offset to center-align with text
   // extra tiles = unknown. compensates for a compiler bug?
   
   gSprites[sMenuState->sprite_id_value_arrow_l].y = y;
   gSprites[sMenuState->sprite_id_value_arrow_r].y = y;
}
static void SetValueArrowVisibility(bool8 visible) {
   gSprites[sMenuState->sprite_id_value_arrow_l].invisible = !visible;
   gSprites[sMenuState->sprite_id_value_arrow_r].invisible = !visible;
}
static void AnimateValueArrows(s8 by) {
   if (by < 0) {
      gSprites[sMenuState->sprite_id_value_arrow_l].sMoving = 1;
   } else {
      gSprites[sMenuState->sprite_id_value_arrow_r].sMoving = 1;
   }
}
//
static void SpriteCB_ValueArrow(struct Sprite* sprite) {
   if (sprite->sMoving) {
      if (++sprite->x2 > VALUE_ARROW_MOVE_DISTANCE) {
         sprite->sMoving = 0;
         sprite->x2      = 0;
      }
   }
}

#undef sDirectionX
#undef sMoving

#undef VALUE_ARROW_MOVE_DISTANCE


static void Task_CGOptionMenuFadeIn(u8 taskId) {
   if (!gPaletteFade.active)
      gTasks[taskId].func = Task_CGOptionMenuProcessInput;
}

static void Task_CGOptionMenuProcessInput(u8 taskId) {
   if (sMenuState->is_in_help) {
      if (JOY_NEW(B_BUTTON)) {
         sMenuState->is_in_help = FALSE;
         HideBg(BACKGROUND_LAYER_HELP);
         UpdateDisplayedControls();
         PlaySE(SOUND_EFFECT_HELP_EXIT);
      }
      return;
   }
   
   // NOTE: We have to check these before the A button, because if the user 
   // sets the "L/R Button" option to "L=A", then pressing L counts as a 
   // press of both L and A.
   if (JOY_NEW(L_BUTTON) || JOY_NEW(R_BUTTON)) {
      const struct CGOptionMenuItem* items = GetCurrentMenuItemList();
      TryDisplayHelp(&items[sMenuState->cursor_pos]);
      return;
   }
   
   if (JOY_NEW(A_BUTTON)) {
      const struct CGOptionMenuItem* items;
      const struct CGOptionMenuItem* item;
      
      items = GetCurrentMenuItemList();
      item  = &items[sMenuState->cursor_pos];
      
      if (item->flags & (1 << MENUITEM_FLAG_IS_SUBMENU)) {
         EnterSubmenu(item->name, item->target.submenu);
         UpdateDisplayedControls();
         HighlightCGOptionMenuItem();
         return;
      }
      
      //
      // TODO: For integral options, pop a number entry box (i.e. let the user type in a number).
      //
      
      //
      // TODO: For Pokemon species, show a special screen to select them.
      //
      return;
   }
   if (JOY_NEW(B_BUTTON)) {
      if (TryExitSubmenu()) {
         UpdateDisplayedControls();
         HighlightCGOptionMenuItem();
         return;
      }
      //
      // We're in the top-level menu. Back out.
      //
      gTasks[taskId].func = Task_CGOptionMenuSave;
      return;
   }
   
   // Up/Down: Move cursor, scrolling if necessary
   if (JOY_NEW(DPAD_UP)) {
      TryMoveMenuCursor(-1);
      UpdateDisplayedMenuItems();
      UpdateDisplayedControls();
      HighlightCGOptionMenuItem();
      return;
   }
   if (JOY_NEW(DPAD_DOWN)) {
      TryMoveMenuCursor(1);
      UpdateDisplayedMenuItems();
      UpdateDisplayedControls();
      HighlightCGOptionMenuItem();
      return;
   }
   
   // Left/Right: Cycle option value
   {
      s8 by = JOY_REPEAT(DPAD_RIGHT) ? 1 : 0;
      if (!by) {
         by = JOY_REPEAT(DPAD_LEFT) ? -1 : 0;
      }
      
      if (by) {
         u8 row;
         const struct CGOptionMenuItem* items = GetCurrentMenuItemList();
         
         row = GetScreenRowForCursorPos();
         
         CycleOptionSelectedValue(&items[sMenuState->cursor_pos], by);
         DrawMenuItem(&items[sMenuState->cursor_pos], row, TRUE);
         
         AnimateValueArrows(by);
         
         PlaySE(SOUND_EFFECT_VALUE_SCROLL);
         
         return;
      }
   }
}

static void Task_CGOptionMenuSave(u8 taskId) {
   gCustomGameOptions = sTempOptions;

   BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
   gTasks[taskId].func = Task_CGOptionMenuFadeOut;
}

static void Task_CGOptionMenuFadeOut(u8 taskId) {
   if (!gPaletteFade.active) {
      DestroyTask(taskId);
      FreeAllWindowBuffers();
      Free(sMenuState);
      sMenuState = NULL;
      SetMainCallback2(gMain.savedCallback);
   }
}


static void TryDisplayHelp(const struct CGOptionMenuItem* item) {
   const u8* text = NULL;
   
   text = GetRelevantHelpText(item);
   if (text == NULL) {
      //
      // No help text to display.
      //
      return;
   }
   
   sMenuState->is_in_help = TRUE;
   
   FillWindowPixelBuffer(WIN_HELP, PIXEL_FILL(1));
   
   StringExpandPlaceholders(gStringVar4, text);
   lu_PrepStringWrap(WIN_HELP, FONT_NORMAL);
   lu_StringWrap(gStringVar4);
   
   AddTextPrinterParameterized3(WIN_HELP, FONT_NORMAL, 2, 1, sTextColor_OptionNames, TEXT_SKIP_DRAW, gStringVar4);
   
   CopyWindowToVram(WIN_HELP, COPYWIN_FULL);
   
   UpdateDisplayedControls();
   ShowBg(BACKGROUND_LAYER_HELP);
}


static void HighlightCGOptionMenuItem() {
   const struct CGOptionMenuItem* item;
   const struct CGOptionMenuItem* items = GetCurrentMenuItemList();
   //
   u8 index = GetScreenRowForCursorPos();
   
   // Draw the cursor indicating the currently focused menu item.
   // (Yes, the black selection indicator in list menus is a text glyph.)
   FillBgTilemapBufferRect(
      BACKGROUND_LAYER_NORMAL,
      offsetof(VRamTileLayout, blank_tile),
      0,
      WIN_OPTIONS_Y_TILES,
      WIN_OPTIONS_X_TILES, // width to paint over
      WIN_OPTIONS_TILE_HEIGHT,
      BACKGROUND_PALETTE_ID_TEXT
   );
   FillBgTilemapBufferRect(
      BACKGROUND_LAYER_NORMAL,
      offsetof(VRamTileLayout, selection_cursor_tiles[0]),
      (WIN_OPTIONS_X_TILES ? WIN_OPTIONS_X_TILES - 1 : 0) / 2,
      WIN_OPTIONS_Y_TILES + (index * 2),
      1,
      1,
      BACKGROUND_PALETTE_ID_TEXT
   );
   FillBgTilemapBufferRect(
      BACKGROUND_LAYER_NORMAL,
      offsetof(VRamTileLayout, selection_cursor_tiles[1]),
      (WIN_OPTIONS_X_TILES ? WIN_OPTIONS_X_TILES - 1 : 0) / 2,
      WIN_OPTIONS_Y_TILES + (index * 2) + 1,
      1,
      1,
      BACKGROUND_PALETTE_ID_TEXT
   );
   CopyBgTilemapBufferToVram(BACKGROUND_LAYER_NORMAL);
   
   // For options (i.e. not submenus, etc.), position arrow indicators around the value. 
   // We'll animate them when the player changes the value, for flavor.
   PositionValueArrowsAtRow(index);
   
   // Of course, for non-options, we need to *hide* those arrows...
   
   item = NULL;
   if (items) {
      item = &items[sMenuState->cursor_pos];
   }
   
   if (item == NULL) {
      SetValueArrowVisibility(FALSE);
      return;
   }
   if ((item->flags & (1 << MENUITEM_FLAG_IS_SUBMENU)) == 0) {
      SetValueArrowVisibility(TRUE);
   } else {
      SetValueArrowVisibility(FALSE);
   }
}

static void UpdateDisplayedMenuName(void) {
   u8 i;
   const u8* title;
   
   title = gText_lu_CGO_menuTitle;
   for(i = 0; i < MAX_MENU_TRAVERSAL_DEPTH; ++i) {
      if (sMenuState->breadcrumbs[i].name != NULL) {
         title = sMenuState->breadcrumbs[i].name;
      } else {
         break;
      }
   }
   
   FillWindowPixelBuffer(WIN_HEADER, PIXEL_FILL(1));
   AddTextPrinterParameterized(WIN_HEADER, FONT_NORMAL, title, 8, 1, TEXT_SKIP_DRAW, NULL);
   CopyWindowToVram(WIN_HEADER, COPYWIN_FULL);
}

static void DrawMenuItem(const struct CGOptionMenuItem* item, u8 row, bool8 is_single_update) {
   u16 value;
   u16 text_width;
   u16 x_offset;
   const u8* value_text;
   
   if (is_single_update) {
      FillWindowPixelRect(
         WIN_OPTIONS,
         PIXEL_FILL(1),
         0,
         (row * 16) + 1,
         
         // Don't paint over the righthand inset, or we'll clobber bits of the scrollbar
         (sOptionMenuWinTemplates[WIN_OPTIONS].width * TILE_WIDTH) - OPTIONS_LIST_INSET_RIGHT,
         
         16
      );
   }
   
   // Name
   AddTextPrinterParameterized3(
      WIN_OPTIONS,
      FONT_NORMAL,
      0, // x
      (row * 16) + 1, // y
      sTextColor_OptionNames,
      TEXT_SKIP_DRAW,
      item->name
   );
   
   // Value
   value      = GetOptionValue(item);
   value_text = GetOptionValueName(item, value);
   if (value_text != NULL) {
      text_width = GetStringWidth(FONT_NORMAL, value_text, 0);
      //
      // let's center the option value within the available space, for now
      // (it'll look neater once we display left/right arrows next to it)
      x_offset = ((s16)OPTION_VALUE_COLUMN_WIDTH - text_width) / 2 + OPTION_VALUE_COLUMN_X;
      //
      AddTextPrinterParameterized3(
         WIN_OPTIONS,
         FONT_NORMAL,
         x_offset,       // x
         (row * 16) + 1, // y
         sTextColor_OptionValues,
         TEXT_SKIP_DRAW,
         value_text
      );
   } else if (item->flags & (1 << MENUITEM_FLAG_IS_SUBMENU)) {
      // TODO: For submenus, draw an icon like ">>" in place of the value
   } else if (item->value_type == VALUE_TYPE_U8 || item->value_type == VALUE_TYPE_U16) {
      
      #define _BUFFER_SIZE 7
      
      u8  text[_BUFFER_SIZE];
      u8  i;
      
      if (value == 0) {
         text[0] = CHAR_0;
         text[1] = EOS;
         if (item->flags & (1 << MENUITEM_FLAG_PERCENTAGE)) {
            text[1] = CHAR_PERCENT;
            text[2] = EOS;
         }
      } else {
         ConvertIntToDecimalStringN(text, value, STR_CONV_MODE_LEFT_ALIGN, _BUFFER_SIZE - 2);
         if (item->flags & (1 << MENUITEM_FLAG_PERCENTAGE)) {
            u16 len = StringLength(text);
            if (len < _BUFFER_SIZE - 2) {
               text[len]     = CHAR_PERCENT;
               text[len + 1] = EOS;
            }
         }
      }
      
      text_width = GetStringWidth(FONT_NORMAL, text, 0);
      x_offset   = ((s16)OPTION_VALUE_COLUMN_WIDTH - text_width) / 2 + OPTION_VALUE_COLUMN_X;
      //
      AddTextPrinterParameterized3(
         WIN_OPTIONS,
         FONT_NORMAL,
         x_offset,       // x
         (row * 16) + 1, // y
         sTextColor_OptionValues,
         TEXT_SKIP_DRAW,
         text
      );
      
      #undef _BUFFER_SIZE
   }
   
   if (is_single_update) {
      //CopyWindowRectToVram(WIN_OPTIONS, COPYWIN_GFX, 0, (row * 16) + 1, sOptionMenuWinTemplates[WIN_OPTIONS].width, 16);
      CopyWindowToVram(WIN_OPTIONS, COPYWIN_GFX);
   }
}

static void GetScrollInformation(u8* pos, u8* count) {
   u8 l_count;
   u8 l_scroll = sMenuState->cursor_pos;
   
   l_count = GetMenuItemListCount(GetCurrentMenuItemList());
   *count = l_count;
   
   if (l_scroll <= MENU_ITEM_HALFWAY_ROW || l_count <= MAX_MENU_ITEMS_VISIBLE_AT_ONCE) {
      l_scroll = 0;
   } else {
      l_scroll -= MENU_ITEM_HALFWAY_ROW;
      if (l_scroll + MAX_MENU_ITEMS_VISIBLE_AT_ONCE > l_count) {
         l_scroll = l_count - MAX_MENU_ITEMS_VISIBLE_AT_ONCE;
      }
   }
   *pos = l_scroll;
}
static u8 GetScrollPosition() {
   u8 count;
   u8 scroll;
   GetScrollInformation(&scroll, &count);
   return scroll;
}
static u8 GetScreenRowForCursorPos(void) {
   u8 count;
   u8 scroll;
   u8 pos = sMenuState->cursor_pos;
   
   count = GetMenuItemListCount(GetCurrentMenuItemList());
   if (count < MAX_MENU_ITEMS_VISIBLE_AT_ONCE) {
      return pos;
   }
   if (pos <= MENU_ITEM_HALFWAY_ROW) {
      return pos;
   }
   scroll = pos - MENU_ITEM_HALFWAY_ROW;
   if (scroll + MAX_MENU_ITEMS_VISIBLE_AT_ONCE > count) {
      scroll = count - MAX_MENU_ITEMS_VISIBLE_AT_ONCE;
   }
   return pos - scroll;
}
static void UpdateDisplayedMenuItems(void) {
   u8 i;
   u8 count;
   u8 scroll;
   const struct CGOptionMenuItem* items;
   
   items  = GetCurrentMenuItemList();
   count  = GetMenuItemListCount(items);
   scroll = GetScrollPosition();
   
   FillWindowPixelBuffer(WIN_OPTIONS, PIXEL_FILL(1));
   
   for(i = 0; i < MAX_MENU_ITEMS_VISIBLE_AT_ONCE; ++i) {
      if (i + scroll >= count) {
         break;
      }
      DrawMenuItem(&items[i + scroll], i, FALSE);
   }
   
   RepaintScrollbar();
   
   CopyWindowToVram(WIN_OPTIONS, COPYWIN_GFX);
}
static void RepaintScrollbar(void) {
   u8 count;
   u8 scroll;
   GetScrollInformation(&scroll, &count);
   
   LuUI_DrawBGScrollbarVert(
      WIN_OPTIONS,
      SCROLLBAR_PALETTE_INDEX_TRACK,
      SCROLLBAR_PALETTE_INDEX_THUMB,
      SCROLLBAR_PALETTE_INDEX_BLANK,
      SCROLLBAR_X - (WIN_OPTIONS_X_TILES * TILE_WIDTH),
      SCROLLBAR_WIDTH,
      0,
      0,
      scroll,
      count,
      MAX_MENU_ITEMS_VISIBLE_AT_ONCE
   );
}
static void UpdateDisplayedControls(void) {
   bool8 draw_help_option;
   //
   const u8* text;
   const u8  color[3] = { TEXT_DYNAMIC_COLOR_6, TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY };

   FillWindowPixelBuffer(WIN_KEYBINDS_STRIP, PIXEL_FILL(15));
   
   draw_help_option = FALSE;
   //
   text = gText_lu_CGO_KeybindsForItem;
   if (sMenuState->is_in_help) {
      text = gText_lu_CGO_KeybindsForHelp;
   } else {
      const struct CGOptionMenuItem* item;
      const struct CGOptionMenuItem* items = GetCurrentMenuItemList();
      
      item = NULL;
      if (items != NULL)
         item = &items[sMenuState->cursor_pos];
      
      if (item) {
         if ((item->flags & (1 << MENUITEM_FLAG_IS_SUBMENU)) != 0) {
            text = gText_lu_CGO_keybindsForSubmenu;
         }
         if (GetRelevantHelpText(item) != NULL) {
            draw_help_option = TRUE;
         }
      }
   }
   AddTextPrinterParameterized3(WIN_KEYBINDS_STRIP, FONT_SMALL, 2, 1, color, TEXT_SKIP_DRAW, text);
   if (draw_help_option) {
      u8 draw_help_option_at = GetStringWidth(FONT_SMALL, text, 0);
      AddTextPrinterParameterized3(WIN_KEYBINDS_STRIP, FONT_SMALL, 2 + draw_help_option_at, 1, color, TEXT_SKIP_DRAW, gText_lu_CGO_keybindFragment_ItemHelp);
   }
   
   CopyWindowToVram(WIN_KEYBINDS_STRIP, COPYWIN_FULL);
}