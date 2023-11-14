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

/*//

   The current menu design is as follows:
   
    - Most of the GUI is on a shared background layer.
    
    - The list of options is on its own background layer. Screen color effects and 
      blending are used to darken all options except the row the player has their 
      cursor on. This is the same basic effect used in the vanilla Options menu. 
      It's set up in `SetUpHighlightEffect`, and the non-darkened area is positioned 
      in `HighlightCGOptionMenuItem`.
      
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
      
    - We should show a scrollbar on the righthand side of the options list. In 
      general, we should eschew the current window frame in favor of custom tiles 
      and palettes. The scrollbar thumb could be drawn as a sprite, potentially 
      with non-uniform scaling if that's supported, to make it easier to animate 
      compared to updating the background layer. Ideally, the scrollbar would 
      only display when the list is long enough to be scrolled.
      
      At present, there's no indicator as to whether the list is scrollable.
      
       - Game Freak has a scrollbar widget like the one we want built into the 
         Pokedex. Could be a good idea to look at it and see how it's coded.
         
          - Things like this make me wonder how many common UI widgets and 
            dialogs could be factored out into reusable functions. Given how 
            low-level a lot of GBA graphics code needs to be, I'm not sure we 
            could do a *ton* with that, but it's worth thinking about. In 
            particular, I'm sure that once we're done with QOL stuff and we 
            start thinking about doing a custom region/story, we'll come up 
            with UI designs that have a lot of shared elements.
      
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

//*/

enum {
   WIN_HEADER,
   WIN_KEYBINDS_STRIP,
   WIN_OPTIONS,
   WIN_HELP,
   //
   WIN_COUNT
};

#define OPTION_VALUE_COLUMN_X     128
#define OPTION_VALUE_COLUMN_WIDTH  94

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
static void UpdateDisplayedControls(void);

static const u16 sOptionMenuText_Pal[] = INCBIN_U16("graphics/interface/option_menu_text.gbapal");
// note: this is only used in the Japanese release
static const u8 sEqualSignGfx[] = INCBIN_U8("graphics/interface/option_menu_equals_sign.4bpp");

static const u8 sTextColor_OptionNames[] = {TEXT_COLOR_WHITE, 6, 7};
static const u8 sTextColor_OptionValues[] = {TEXT_COLOR_WHITE, TEXT_COLOR_RED, TEXT_COLOR_LIGHT_RED};
static const u8 sTextColor_HelpBodyText[] = {TEXT_COLOR_WHITE, 6, 7};

#define BACKGROUND_LAYER_NORMAL  0
#define BACKGROUND_LAYER_OPTIONS 1
#define BACKGROUND_LAYER_HELP    2

#define BACKGROUND_PALETTE_ID_MENU     0
#define BACKGROUND_PALETTE_ID_TEXT     1
#define BACKGROUND_PALETTE_ID_CONTROLS 2
#define BACKGROUND_PALETTE_BOX_FRAME   7

#define DIALOG_FRAME_FIRST_TILE_ID 485
//
#define DIALOG_FRAME_TILE_TOP_CORNER_L (DIALOG_FRAME_FIRST_TILE_ID)
#define DIALOG_FRAME_TILE_TOP_EDGE     (DIALOG_FRAME_FIRST_TILE_ID + 1)
#define DIALOG_FRAME_TILE_TOP_CORNER_R (DIALOG_FRAME_FIRST_TILE_ID + 2)
#define DIALOG_FRAME_TILE_LEFT_EDGE    (DIALOG_FRAME_FIRST_TILE_ID + 3)
#define DIALOG_FRAME_TILE_CENTER       (DIALOG_FRAME_FIRST_TILE_ID + 4)
#define DIALOG_FRAME_TILE_RIGHT_EDGE   (DIALOG_FRAME_FIRST_TILE_ID + 5)
#define DIALOG_FRAME_TILE_BOT_CORNER_L (DIALOG_FRAME_FIRST_TILE_ID + 6)
#define DIALOG_FRAME_TILE_BOT_EDGE     (DIALOG_FRAME_FIRST_TILE_ID + 7)
#define DIALOG_FRAME_TILE_BOT_CORNER_R (DIALOG_FRAME_FIRST_TILE_ID + 8)

#define OPTIONS_LIST_TILE_HEIGHT    14
#define OPTIONS_LIST_PIXEL_HEIGHT   (OPTIONS_LIST_TILE_HEIGHT * TILE_HEIGHT)
//
#define MAX_MENU_ITEMS_VISIBLE_AT_ONCE   OPTIONS_LIST_TILE_HEIGHT / 2
#define MENU_ITEM_HALFWAY_ROW            (MAX_MENU_ITEMS_VISIBLE_AT_ONCE / 2)

#define SCROLLBAR_X     230
#define SCROLLBAR_WIDTH   3
#define SCROLLBAR_TRACK_HEIGHT   OPTIONS_LIST_PIXEL_HEIGHT

static const struct WindowTemplate sOptionMenuWinTemplates[] = {
    [WIN_HEADER] = {
        .bg          = BACKGROUND_LAYER_NORMAL,
        .tilemapLeft = 0,
        .tilemapTop  = 0,
        .width       = DISPLAY_TILE_WIDTH,
        .height      = 2,
        .paletteNum  = BACKGROUND_PALETTE_ID_CONTROLS,
        .baseBlock   = 1
    },
    [WIN_KEYBINDS_STRIP] = {
        .bg          = BACKGROUND_LAYER_NORMAL,
        .tilemapLeft = 0,
        .tilemapTop  = DISPLAY_TILE_HEIGHT - 2,
        .width       = DISPLAY_TILE_WIDTH,
        .height      = 2,
        .paletteNum  = BACKGROUND_PALETTE_ID_CONTROLS,
        .baseBlock   = 1 + (DISPLAY_TILE_WIDTH * 2)
    },
    [WIN_OPTIONS] = {
        .bg          = BACKGROUND_LAYER_OPTIONS,
        .tilemapLeft = 2,
        .tilemapTop  = 3,
        .width       = 26,
        .height      = OPTIONS_LIST_TILE_HEIGHT,
        .paletteNum  = BACKGROUND_PALETTE_ID_TEXT,
        .baseBlock   = 1 + (DISPLAY_TILE_WIDTH * 4)
    },
    [WIN_HELP] = {
        .bg          = BACKGROUND_LAYER_HELP,
        .tilemapLeft = 0,
        .tilemapTop  = 2,
        .width       = DISPLAY_TILE_WIDTH,
        .height      = DISPLAY_TILE_HEIGHT - 4,
        .paletteNum  = BACKGROUND_PALETTE_ID_TEXT,
        .baseBlock   = 1 // this is in its own background with its own tiles, so it can use the same number as other windows
    },
    //
    [WIN_COUNT] = DUMMY_WIN_TEMPLATE
};

static const struct BgTemplate sOptionMenuBgTemplates[] = {
   {
      .bg = BACKGROUND_LAYER_NORMAL,
      //
      .charBaseIndex = 1,
      .mapBaseIndex  = 31,
      .screenSize    = 0,
      .paletteMode   = 0,
      .priority      = 2,
      .baseTile      = 0
   },
   {
      .bg = BACKGROUND_LAYER_OPTIONS,
      //
      .charBaseIndex = 1,
      .mapBaseIndex  = 30,
      .screenSize    = 0,
      .paletteMode   = 0,
      .priority      = 1,
      .baseTile      = 0
   },
   {
      .bg = BACKGROUND_LAYER_HELP,
      //
      .charBaseIndex = 2,
      .mapBaseIndex  = 32,
      .screenSize    = 0,
      .paletteMode   = 0,
      .priority      = 0,
      .baseTile      = 0
   },
};

static const u16 sOptionMenuBg_Pal[] = {RGB(17, 18, 31)};

#define SPRITE_TAG_COALESCED_INTERFACE 0x1000 // Tile and pal tag used for all interface sprites.

static const struct CompressedSpriteSheet sInterfaceSpriteSheet = {
   gLuCGOMenuInterface_Gfx,
   8*8 / 2, // uncompressed size of pixel data (width times height), divided by two
   SPRITE_TAG_COALESCED_INTERFACE
};
static const struct SpritePalette sInterfaceSpritePalette[] = {
   {gLuCGOMenuInterface_Pal, SPRITE_TAG_COALESCED_INTERFACE},
   {0}
};

#define SCROLLBAR_THUMB_OAM_MATRIX_INDEX 0

static const struct OamData sOamData_ScrollbarThumb = {
   .y           = DISPLAY_HEIGHT,
   .affineMode  = ST_OAM_AFFINE_NORMAL,
   .objMode     = ST_OAM_OBJ_NORMAL,
   .mosaic      = FALSE,
   .bpp         = ST_OAM_4BPP,
   .shape       = SPRITE_SHAPE(8x8),
   .x           = 0,
   .matrixNum   = SCROLLBAR_THUMB_OAM_MATRIX_INDEX, // relevant for calls to SetOamMatrix
   .size        = SPRITE_SIZE(8x8),
   .tileNum     = 0,
   .priority    = 1,
   .paletteNum  = 0,
   .affineParam = 0
};
static const union AnimCmd sSpriteAnim_ScrollbarThumb[] = {
   ANIMCMD_FRAME(0, 30),
   ANIMCMD_END
};
static const union AnimCmd* const sSpriteAnimTable_ScrollbarThumb[] = {
   sSpriteAnim_ScrollbarThumb
};
static const struct SpriteTemplate sScrollbarThumbSpriteTemplate = {
   .tileTag     = SPRITE_TAG_COALESCED_INTERFACE,
   .paletteTag  = SPRITE_TAG_COALESCED_INTERFACE,
   .oam         = &sOamData_ScrollbarThumb,
   .anims       = sSpriteAnimTable_ScrollbarThumb,
   .images      = NULL,
   .affineAnims = gDummySpriteAffineAnimTable,
   .callback    = SpriteCB_ScrollbarThumb,
};


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
   
   // Once we enable an LCD I/O window region, we have to explicitly define which 
   // background layers render in which regions, no matter what else we've done to 
   // enable a background layer. In this case, we want all layers visible; the only 
   // difference between regions is whether they have the GBA color effect applied.\
   //
   // Oh -- we also have to enable the object layer, too!
   
   SetGpuReg(REG_OFFSET_WIN0H,  0);
   SetGpuReg(REG_OFFSET_WIN0V,  0);
   SetGpuReg(REG_OFFSET_WININ,  WINOUT_WIN01_BG_ALL);
   SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_ALL);
   SetGpuReg(REG_OFFSET_BLDCNT, (1 << BACKGROUND_LAYER_OPTIONS) | BLDCNT_EFFECT_DARKEN);
   SetGpuReg(REG_OFFSET_BLDALPHA, 0);
   SetGpuReg(REG_OFFSET_BLDY, 4);
   SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_WIN0_ON | DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
   
   #undef _ALL_BG_FLAGS
}

void CB2_InitCustomGameOptionMenu(void) {
   switch (gMain.state) {
      default:
      case 0:
         SetVBlankCallback(NULL);
         gMain.state++;
         break;
      case 1:
         //
         // Clear VRAM, OAM, and palettes, and reset some VRAM data.
         //
         DmaClearLarge16(3, (void *)(VRAM), VRAM_SIZE, 0x1000);
         DmaClear32(3, OAM, OAM_SIZE);
         DmaClear16(3, PLTT, PLTT_SIZE);
         SetGpuReg(REG_OFFSET_DISPCNT, 0);
         ResetBgsAndClearDma3BusyFlags(0);
         //
         // Load our backgrounds, and reset all four background layers' positions.
         //
         InitBgsFromTemplates(0, sOptionMenuBgTemplates, ARRAY_COUNT(sOptionMenuBgTemplates));
         ChangeBgX(0, 0, BG_COORD_SET);
         ChangeBgY(0, 0, BG_COORD_SET);
         ChangeBgX(1, 0, BG_COORD_SET);
         ChangeBgY(1, 0, BG_COORD_SET);
         ChangeBgX(2, 0, BG_COORD_SET);
         ChangeBgY(2, 0, BG_COORD_SET);
         ChangeBgX(3, 0, BG_COORD_SET);
         ChangeBgY(3, 0, BG_COORD_SET);
           
         InitWindows(sOptionMenuWinTemplates);
         DeactivateAllTextPrinters();
         
         SetUpHighlightEffect();
         
         ShowBg(BACKGROUND_LAYER_NORMAL);
         ShowBg(BACKGROUND_LAYER_OPTIONS);
         HideBg(BACKGROUND_LAYER_HELP);
         
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
         ResetPaletteFade();
         ScanlineEffect_Stop();
         ResetTasks();
         ResetSpriteData();
         FreeAllSpritePalettes();
         {
            LoadCompressedSpriteSheet(&sInterfaceSpriteSheet);
            LoadSpritePalettes(sInterfaceSpritePalette);
            CreateSprite(&sScrollbarThumbSpriteTemplate, SCROLLBAR_X, (sOptionMenuWinTemplates[WIN_OPTIONS].tilemapTop * TILE_HEIGHT), 0);
         }
         gMain.state++;
         break;
       case 3:
         LoadBgTiles(1, GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->tiles, 0x120, DIALOG_FRAME_FIRST_TILE_ID);
         gMain.state++;
         break;
       case 4:
         LoadPalette(sOptionMenuBg_Pal, BG_PLTT_ID(BACKGROUND_PALETTE_ID_MENU), sizeof(sOptionMenuBg_Pal));
         LoadPalette(GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->pal, BG_PLTT_ID(BACKGROUND_PALETTE_BOX_FRAME), PLTT_SIZE_4BPP);
         gMain.state++;
         break;
       case 5:
         LoadPalette(sOptionMenuText_Pal, BG_PLTT_ID(BACKGROUND_PALETTE_ID_TEXT), sizeof(sOptionMenuText_Pal));
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
      // TODO: For Pokemon species, show a special screen to select them
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
   u8 index = GetScreenRowForCursorPos();
   
   SetGpuReg(REG_OFFSET_WIN0H, WIN_RANGE(16, DISPLAY_WIDTH - 16));
   SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(index * 16 + 24, index * 16 + 40));
   
   //
   // TODO: Show left/right arrows around option value
   // TODO: Change left/right arrow icons depending on user is at minimum value
   //        - Don't grey them out, because they wrap around. Maybe shrink them?
   //
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
      FillWindowPixelRect(WIN_OPTIONS, PIXEL_FILL(1), 0, (row * 16) + 1, sOptionMenuWinTemplates[WIN_OPTIONS].width * TILE_WIDTH, 16);
   }
   
   // Name
   AddTextPrinterParameterized3(
      WIN_OPTIONS,
      FONT_NORMAL,
      8,              // x
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
      x_offset = (OPTION_VALUE_COLUMN_WIDTH - text_width) / 2 + OPTION_VALUE_COLUMN_X;
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
      x_offset   = (OPTION_VALUE_COLUMN_WIDTH - text_width) / 2 + OPTION_VALUE_COLUMN_X;
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
   
   CopyWindowToVram(WIN_OPTIONS, COPYWIN_GFX);
}

static void SpriteCB_ScrollbarThumb(struct Sprite* sprite) {
   u8 scroll;
   u8 count;
   GetScrollInformation(&scroll, &count);
   
   if (count <= MAX_MENU_ITEMS_VISIBLE_AT_ONCE) {
      sprite->invisible = TRUE;
      return;
   }
   sprite->invisible = FALSE;
   
   {
      // thumb size / scroll bar size = page size / scroll bar range
      //    ergo
      // thumb size = scroll bar size * page size / scroll bar range
      s16 scale;
      
      u8 offset = SCROLLBAR_TRACK_HEIGHT * scroll / count;
      
      //
      // Fun fact: OAM matrices are the inverse of standard transformation matrices!
      // Where an intuitive matrix would map texels to screen pixels, OAM matrices 
      // instead map screen pixels to texels!
      //
      // The scale we want is:
      //
      //    SCROLLBAR_TRACK_HEIGHT * (MAX_MENU_ITEMS_VISIBLE_AT_ONCE / count)
      //
      // If OAM matrices were, uh, sane, then we'd do:
      //
      //    (SCROLLBAR_TRACK_HEIGHT * (MAX_MENU_ITEMS_VISIBLE_AT_ONCE / count)) * 0x100
      //
      // So what we actually need to do is (1/N)*0x100, i.e.
      //
      //    0x100 * count / SCROLLBAR_TRACK_HEIGHT / MAX_MENU_ITEMS_VISIBLE_AT_ONCE
      //
      // And that would work with a 1px image, except that it wouldn't be granular 
      // enough. If we use an 8px-tall thumb sprite, then we can multiply this whole 
      // value by 8 to gain more range.
      //
      
      scale = 0x800 * count / (SCROLLBAR_TRACK_HEIGHT * MAX_MENU_ITEMS_VISIBLE_AT_ONCE);
      if (scale > 0x800) // similarly, because these matrices are inverted, we want to enforce a maximum
         scale = 0x800;
      if (scale <= 0) // but we still need to guard against zero-or-negative values
         scale = 0x800;
scale = 0x10; // TEST
      
      SetOamMatrix(SCROLLBAR_THUMB_OAM_MATRIX_INDEX, 0x100, 0, 0, scale);
      
      sprite->y2 = offset;
   }
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