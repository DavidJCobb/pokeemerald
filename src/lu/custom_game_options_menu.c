#include "global.h"
#include "lu/custom_game_options_menu.h"
#include "lu/custom_game_options.h"

#include "bg.h"
#include "gpu_regs.h"
#include "main.h"
#include "malloc.h" // AllocZeroed, Free
#include "menu.h"
#include "palette.h"
#include "scanline_effect.h"
#include "sprite.h"
#include "strings.h" // literally just for the "Cancel" option lol
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "gba/m4a_internal.h"
#include "constants/rgb.h"

#include "characters.h"
#include "string_util.h"
#include "lu/strings.h"

enum {
   WIN_HEADER,
   WIN_KEYBINDS_STRIP,
   WIN_OPTIONS,
   //
   WIN_COUNT
};

#define MAX_MENU_ITEMS_VISIBLE_AT_ONCE 7
//
#define MENU_ITEM_HALFWAY_ROW (MAX_MENU_ITEMS_VISIBLE_AT_ONCE / 2)

#define OPTION_VALUE_COLUMN_X     128
#define OPTION_VALUE_COLUMN_WIDTH  94

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
   u8 cursor_pos;
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
}

//
// Task and drawing stuff:
//

static void Task_CGOptionMenuFadeIn(u8 taskId);
static void Task_CGOptionMenuProcessInput(u8 taskId);
static void Task_CGOptionMenuSave(u8 taskId);
static void Task_CGOptionMenuFadeOut(u8 taskId);
static void HighlightCGOptionMenuItem();

static void UpdateDisplayedMenuName(void);
static void DrawMenuItem(const struct CGOptionMenuItem* item, u8 row);
static void UpdateDisplayedMenuItems(void);

static void DrawControls(void);
static void DrawBgWindowFrames(void);

static const u16 sOptionMenuText_Pal[] = INCBIN_U16("graphics/interface/option_menu_text.gbapal");
// note: this is only used in the Japanese release
static const u8 sEqualSignGfx[] = INCBIN_U8("graphics/interface/option_menu_equals_sign.4bpp");

static const u8 sTextColor_OptionNames[] = {TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};
static const u8 sTextColor_OptionValues[] = {TEXT_COLOR_WHITE, TEXT_COLOR_RED, TEXT_COLOR_LIGHT_RED};

#define BACKGROUND_LAYER_NORMAL  0
#define BACKGROUND_LAYER_OPTIONS 1

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

static const struct WindowTemplate sOptionMenuWinTemplates[] = {
    [WIN_HEADER] = {
        .bg          = BACKGROUND_LAYER_NORMAL,
        .tilemapLeft = 0,
        .tilemapTop  = 2,
        .width       = DISPLAY_TILE_WIDTH,
        .height      = 2,
        .paletteNum  = BACKGROUND_PALETTE_ID_CONTROLS,
        .baseBlock   = 1
    },
    [WIN_KEYBINDS_STRIP] = {
        .bg          = BACKGROUND_LAYER_NORMAL,
        .tilemapLeft = 0,
        .tilemapTop  = 0,
        .width       = DISPLAY_TILE_WIDTH,
        .height      = 2,
        .paletteNum  = BACKGROUND_PALETTE_ID_CONTROLS,
        .baseBlock   = 1 + (DISPLAY_TILE_WIDTH * 2)
    },
    [WIN_OPTIONS] = {
        .bg          = BACKGROUND_LAYER_OPTIONS,
        .tilemapLeft = 2,
        .tilemapTop  = 5,
        .width       = 26,
        .height      = 14,
        .paletteNum  = BACKGROUND_PALETTE_ID_TEXT,
        .baseBlock   = 1 + (DISPLAY_TILE_WIDTH * 4)
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
      .screenSize    = 0, // 256x256px
      .paletteMode   = 0, // 16 palettes with 16 colors per
      .priority      = 1,
      .baseTile      = 0
   },
   {
      .bg = BACKGROUND_LAYER_OPTIONS,
      //
      .charBaseIndex = 1,
      .mapBaseIndex  = 30,
      .screenSize    = 0, // 256x256px
      .paletteMode   = 0, // 16 palettes with 16 colors per
      .priority      = 0,
      .baseTile      = 0
   },
};

static const u16 sOptionMenuBg_Pal[] = {RGB(17, 18, 31)};

static void MainCB2(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void VBlankCB(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
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
         SetGpuReg(REG_OFFSET_WIN0H,  0);
         SetGpuReg(REG_OFFSET_WIN0V,  0);
         SetGpuReg(REG_OFFSET_WININ,  (1 << BACKGROUND_LAYER_OPTIONS));
         SetGpuReg(REG_OFFSET_WINOUT, (1 << BACKGROUND_LAYER_NORMAL) | (1 << BACKGROUND_LAYER_OPTIONS) | WINOUT_WIN01_CLR);
         SetGpuReg(REG_OFFSET_BLDCNT, (1 << BACKGROUND_LAYER_OPTIONS) | BLDCNT_EFFECT_DARKEN);
         SetGpuReg(REG_OFFSET_BLDALPHA, 0);
         SetGpuReg(REG_OFFSET_BLDY, 4);
         SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_WIN0_ON | DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
         
         ShowBg(0);
         ShowBg(1);
           
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
         DrawControls();
         DrawBgWindowFrames();
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
   if (JOY_NEW(A_BUTTON)) {
      const struct CGOptionMenuItem* items;
      const struct CGOptionMenuItem* item;
      
      items = GetCurrentMenuItemList();
      item  = &items[sMenuState->cursor_pos];
      
      if (item->flags & (1 << MENUITEM_FLAG_IS_SUBMENU)) {
         EnterSubmenu(item->name, item->target.submenu);
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
         HighlightCGOptionMenuItem();
         return;
      }
      //
      // We're in the top-level menu. Back out.
      //
      gTasks[taskId].func = Task_CGOptionMenuSave;
      return;
   }
   if (JOY_NEW(L_BUTTON) || JOY_NEW(R_BUTTON)) {
      //
      // TODO: Show help text for current option/value pair. Use my automatic word-wrap 
      // code, since we aren't putting line breaks in these strings.
      //
      return;
   }
   
   // Up/Down: Move cursor, scrolling if necessary
   if (JOY_NEW(DPAD_UP)) {
      TryMoveMenuCursor(-1);
      UpdateDisplayedMenuItems();
      HighlightCGOptionMenuItem();
      return;
   }
   if (JOY_NEW(DPAD_DOWN)) {
      TryMoveMenuCursor(1);
      UpdateDisplayedMenuItems();
      HighlightCGOptionMenuItem();
      return;
   }
   
   // Left/Right: Cycle option value
   {
      s8 by = JOY_NEW(DPAD_RIGHT) ? 1 : 0;
      if (!by) {
         by = JOY_NEW(DPAD_LEFT) ? -1 : 0;
      }
      if (by) {
         u8 row;
         const struct CGOptionMenuItem* items = GetCurrentMenuItemList();
         
         row = GetScreenRowForCursorPos();
         
         CycleOptionSelectedValue(&items[sMenuState->cursor_pos], by);
         //FillWindowPixelRect(WIN_OPTIONS, PIXEL_FILL(1), 8, (row * 16) + 1, 240, 16);
         DrawMenuItem(&items[sMenuState->cursor_pos], row);
         CopyWindowToVram(WIN_OPTIONS, COPYWIN_GFX);
         //CopyWindowRectToVram(WIN_OPTIONS, COPYWIN_GFX, 8, (row * 16) + 1, 240, 16);
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

static void HighlightCGOptionMenuItem() {
   u8 index = GetScreenRowForCursorPos();
   
   SetGpuReg(REG_OFFSET_WIN0H, WIN_RANGE(16, DISPLAY_WIDTH - 16));
   SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(index * 16 + 40, index * 16 + 56));
   
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

static void DrawMenuItem(const struct CGOptionMenuItem* item, u8 row) {
   u16 value;
   u16 text_width;
   u16 x_offset;
   const u8* value_text;
   
   FillWindowPixelRect(WIN_OPTIONS, PIXEL_FILL(1), 8, (row * 16) + 1, 240 - 8, 16);
   
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
      u8  text[7];
      u8  i;
      
      if (value == 0) {
         text[0] = CHAR_0;
         text[1] = EOS;
      } else {
         ConvertIntToDecimalStringN(text, value, STR_CONV_MODE_LEFT_ALIGN, 6);
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
   }
}

static u8 GetScrollPosition() {
   u8 count;
   u8 scroll = sMenuState->cursor_pos;
   if (scroll <= MENU_ITEM_HALFWAY_ROW) {
      scroll = 0;
   } else {
      scroll -= MENU_ITEM_HALFWAY_ROW;
      if (count >= MAX_MENU_ITEMS_VISIBLE_AT_ONCE) {
         if (scroll + MAX_MENU_ITEMS_VISIBLE_AT_ONCE > count) {
            scroll = count - MAX_MENU_ITEMS_VISIBLE_AT_ONCE;
         }
      }
   }
   return scroll;
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
      DrawMenuItem(&items[i + scroll], i);
   }
   
   // 100% necessary or we see a blank window when we scroll
   CopyWindowToVram(WIN_OPTIONS, COPYWIN_GFX);
}
static u8 GetScreenRowForCursorPos(void) {
   u8 pos    = sMenuState->cursor_pos;
   u8 scroll = GetScrollPosition();
   return pos - scroll;
}

static void DrawBgWindowFrames(void) {
    //                     bg, tile,              x, y, width, height, palNum
    /*//
    // Draw title window frame
    FillBgTilemapBufferRect(BACKGROUND_LAYER_NORMAL, DIALOG_FRAME_TILE_TOP_CORNER_L,  1,  0,  1,  1,  BACKGROUND_PALETTE_BOX_FRAME);
    FillBgTilemapBufferRect(BACKGROUND_LAYER_NORMAL, DIALOG_FRAME_TILE_TOP_EDGE,      2,  0, 27,  1,  BACKGROUND_PALETTE_BOX_FRAME);
    FillBgTilemapBufferRect(BACKGROUND_LAYER_NORMAL, DIALOG_FRAME_TILE_TOP_CORNER_R, 28,  0,  1,  1,  BACKGROUND_PALETTE_BOX_FRAME);
    FillBgTilemapBufferRect(BACKGROUND_LAYER_NORMAL, DIALOG_FRAME_TILE_LEFT_EDGE,     1,  1,  1,  2,  BACKGROUND_PALETTE_BOX_FRAME);
    FillBgTilemapBufferRect(BACKGROUND_LAYER_NORMAL, DIALOG_FRAME_TILE_RIGHT_EDGE,   28,  1,  1,  2,  BACKGROUND_PALETTE_BOX_FRAME);
    FillBgTilemapBufferRect(BACKGROUND_LAYER_NORMAL, DIALOG_FRAME_TILE_BOT_CORNER_L,  1,  3,  1,  1,  BACKGROUND_PALETTE_BOX_FRAME);
    FillBgTilemapBufferRect(BACKGROUND_LAYER_NORMAL, DIALOG_FRAME_TILE_BOT_EDGE,      2,  3, 27,  1,  BACKGROUND_PALETTE_BOX_FRAME);
    FillBgTilemapBufferRect(BACKGROUND_LAYER_NORMAL, DIALOG_FRAME_TILE_BOT_CORNER_R, 28,  3,  1,  1,  BACKGROUND_PALETTE_BOX_FRAME);
    //*/

    // Draw options list window frame
    FillBgTilemapBufferRect(BACKGROUND_LAYER_NORMAL, DIALOG_FRAME_TILE_TOP_CORNER_L,  1,  4,  1,  1,  BACKGROUND_PALETTE_BOX_FRAME);
    FillBgTilemapBufferRect(BACKGROUND_LAYER_NORMAL, DIALOG_FRAME_TILE_TOP_EDGE,      2,  4, 26,  1,  BACKGROUND_PALETTE_BOX_FRAME);
    FillBgTilemapBufferRect(BACKGROUND_LAYER_NORMAL, DIALOG_FRAME_TILE_TOP_CORNER_R, 28,  4,  1,  1,  BACKGROUND_PALETTE_BOX_FRAME);
    FillBgTilemapBufferRect(BACKGROUND_LAYER_NORMAL, DIALOG_FRAME_TILE_LEFT_EDGE,     1,  5,  1, 18,  BACKGROUND_PALETTE_BOX_FRAME);
    FillBgTilemapBufferRect(BACKGROUND_LAYER_NORMAL, DIALOG_FRAME_TILE_RIGHT_EDGE,   28,  5,  1, 18,  BACKGROUND_PALETTE_BOX_FRAME);
    FillBgTilemapBufferRect(BACKGROUND_LAYER_NORMAL, DIALOG_FRAME_TILE_BOT_CORNER_L,  1, 19,  1,  1,  BACKGROUND_PALETTE_BOX_FRAME);
    FillBgTilemapBufferRect(BACKGROUND_LAYER_NORMAL, DIALOG_FRAME_TILE_BOT_EDGE,      2, 19, 26,  1,  BACKGROUND_PALETTE_BOX_FRAME);
    FillBgTilemapBufferRect(BACKGROUND_LAYER_NORMAL, DIALOG_FRAME_TILE_BOT_CORNER_R, 28, 19,  1,  1,  BACKGROUND_PALETTE_BOX_FRAME);

    CopyBgTilemapBufferToVram(BACKGROUND_LAYER_NORMAL);
}

static void DrawControls(void) {
   const u8 color[3] = { TEXT_DYNAMIC_COLOR_6, TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY };

   FillWindowPixelBuffer(WIN_KEYBINDS_STRIP, PIXEL_FILL(15));
   AddTextPrinterParameterized3(WIN_KEYBINDS_STRIP, FONT_SMALL, 2, 1, color, TEXT_SKIP_DRAW, gText_lu_CGO_keybinds);
   CopyWindowToVram(WIN_KEYBINDS_STRIP, COPYWIN_FULL);
}