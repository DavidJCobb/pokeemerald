#include "global.h"
#include "lu/custom_game_options_menu.h"
#include "lu/custom_game_options.h"

#include "bg.h"
#include "gpu_regs.h"
#include "main.h"
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

#define tMenuSelection data[0]
#define tMenuScrollOffset data[1]

#include "characters.h"
#include "lu/strings.h"

enum {
   WIN_HEADER,
   WIN_OPTIONS
};

#define MENUITEM_COUNT 2
#define MENUITEM_CANCEL (MENUITEM_COUNT - 1)

#define MAX_VALUES_PER_MENU_ITEM 6
#define MAX_MENU_ITEMS_VISIBLE_AT_ONCE 7
//
#define MENU_ITEM_HALFWAY_ROW (MAX_MENU_ITEMS_VISIBLE_AT_ONCE / 2)

EWRAM_DATA static struct CustomGameOptions sTempOptions;

enum {
   VALUE_TYPE_NONE, // e.g. "Cancel"
   VALUE_TYPE_U8,
   VALUE_TYPE_U16,
   VALUE_TYPE_BOOL8,
};

//
// This array stores definitions for each menu item, including "Cancel." Be aware that 
// at present, we store the current state of each menu item in the menu's task data, 
// which has 16 fields -- two of which are currently used by the menu itself. This 
// means that we can define up to 14 menu items, including "Cancel," before we need 
// to either start storing that data somewhere else, or start storing it more cleverly 
// (e.g. the fields are u16s but the options hold u8s, so we could double storage with 
// some bitshifting shenanigans).
//
static const struct CGOptionMenuItem {
   // Option name
   const u8* name;
   const u8* value_strings[MAX_VALUES_PER_MENU_ITEM]; // NULL to terminate the list early
   union {
      bool8* as_bool8;
      u8*    as_u8;
      u16*   as_u16;
   } target;
   u8 value_type;
} sOptions[MENUITEM_COUNT] = {
   {
      .name          = gText_lu_CGOption_startWithRunningShoes,
      .value_strings = {
         gText_lu_CGOptionValues_common_Disabled,
         gText_lu_CGOptionValues_common_Enabled,
         NULL
      },
      .target     = { .as_u8 = &sTempOptions.qol.start_with_running_shoes },
      .value_type = VALUE_TYPE_BOOL8,
   },
   [MENUITEM_CANCEL] = {
      .name          = gText_OptionMenuCancel,
      .value_strings = { NULL },
      .target        = { .as_u8 = NULL },
      .value_type    = VALUE_TYPE_NONE,
   },
};

static void Task_CGOptionMenuFadeIn(u8 taskId);
static void ScrollCGOptionMenuTo(u8 taskId, u8 to);
static void ScrollCGOptionMenuBy(u8 taskId, s8 by);
static void Task_CGOptionMenuProcessInput(u8 taskId);
static void Task_CGOptionMenuSave(u8 taskId);
static void Task_CGOptionMenuFadeOut(u8 taskId);
static void HighlightCGOptionMenuItem(u8 selection);

// We take pointers here rather than row/option indices because the plan is to eventually 
// have nested submenus and whatnot.
static u16  GetOptionSelectedValueIndex(const struct CGOptionMenuItem* item);
static void SetOptionSelectedValueIndex(const struct CGOptionMenuItem* item, u16 selectionIndex);
static u8   GetOptionValueCount(const struct CGOptionMenuItem* item);
static void CycleOptionSelectedValue(const struct CGOptionMenuItem* item, s8 by);

static void DrawFullOptionRow(const struct CGOptionMenuItem* item, u8 visibleRow);
static void DrawOptionName(const struct CGOptionMenuItem* item, u8 visibleRow);
static void DrawOptionRowValue(const struct CGOptionMenuItem* item, u8 visibleRow);
static void DrawHeaderText(void); // draw menu title
static void DrawBgWindowFrames(void);

// Options' "process inputs" functions should set this to TRUE if the option's value 
// has changed. Updates to VRAM are done conditionally based on both changes to the 
// value (as detected from outside the "process inputs" function) and sArrowPressed.
EWRAM_DATA static bool8 sArrowPressed = FALSE;

static const u16 sOptionMenuText_Pal[] = INCBIN_U16("graphics/interface/option_menu_text.gbapal");
// note: this is only used in the Japanese release
static const u8 sEqualSignGfx[] = INCBIN_U8("graphics/interface/option_menu_equals_sign.4bpp");

static u16 GetOptionSelectedValueIndex(const struct CGOptionMenuItem* item) {
   switch (item->value_type) {
      case VALUE_TYPE_U8:
         return *(item->target.as_u8);
      case VALUE_TYPE_U16:
         return *(item->target.as_u16);
      case VALUE_TYPE_BOOL8:
         return *(item->target.as_bool8) != 0 ? 1 : 0;
   }
   return 0;
}
static void SetOptionSelectedValueIndex(const struct CGOptionMenuItem* item, u16 selectionIndex) {
   switch (item->value_type) {
      case VALUE_TYPE_U8:
         *(item->target.as_u8) = selectionIndex;
         return;
      case VALUE_TYPE_U16:
         *(item->target.as_u16) = selectionIndex;
         return;
      case VALUE_TYPE_BOOL8:
         *(item->target.as_bool8) = (selectionIndex != 0);
         return;
   }
   return;
}
static u8 GetOptionValueCount(const struct CGOptionMenuItem* item) {
   u8 value_count = 0;
   while (TRUE) {
      if (!item->value_strings[value_count])
         break;
      ++value_count;
      if (value_count == MAX_VALUES_PER_MENU_ITEM)
         break;
   }
   return value_count;
}
static void CycleOptionSelectedValue(const struct CGOptionMenuItem* item, s8 by) {
   s8 selection;
   u8 value_count;
   
   selection   = (s8) GetOptionSelectedValueIndex(item) + by;
   value_count = GetOptionValueCount(item);
   
   switch (item->value_type) {
      case VALUE_TYPE_U8:
         selection %= value_count;
         break;
      case VALUE_TYPE_U16:
         selection %= value_count;
         break;
      case VALUE_TYPE_BOOL8:
         selection = !(selection & 1);
         break;
   }
   
   SetOptionSelectedValueIndex(item, selection);
}

static const struct WindowTemplate sOptionMenuWinTemplates[] =
{
    [WIN_HEADER] = {
        .bg = 1,
        .tilemapLeft = 2,
        .tilemapTop = 1,
        .width = 26,
        .height = 2,
        .paletteNum = 1,
        .baseBlock = 2
    },
    [WIN_OPTIONS] = {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 5,
        .width = 26,
        .height = 14,
        .paletteNum = 1,
        .baseBlock = 0x36
    },
    DUMMY_WIN_TEMPLATE
};

static const struct BgTemplate sOptionMenuBgTemplates[] =
{
    {
        .bg = 1,
        .charBaseIndex = 1,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    },
    {
        .bg = 0,
        .charBaseIndex = 1,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0
    }
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
           DmaClearLarge16(3, (void *)(VRAM), VRAM_SIZE, 0x1000);
           DmaClear32(3, OAM, OAM_SIZE);
           DmaClear16(3, PLTT, PLTT_SIZE);
           SetGpuReg(REG_OFFSET_DISPCNT, 0);
           ResetBgsAndClearDma3BusyFlags(0);
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
           SetGpuReg(REG_OFFSET_WIN0H, 0);
           SetGpuReg(REG_OFFSET_WIN0V, 0);
           SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_BG0);
           SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_BG0 | WINOUT_WIN01_BG1 | WINOUT_WIN01_CLR);
           SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT1_BG0 | BLDCNT_EFFECT_DARKEN);
           SetGpuReg(REG_OFFSET_BLDALPHA, 0);
           SetGpuReg(REG_OFFSET_BLDY, 4);
           SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_WIN0_ON | DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
           ShowBg(0);
           ShowBg(1);
           gMain.state++;
           break;
       case 2:
           ResetPaletteFade();
           ScanlineEffect_Stop();
           ResetTasks();
           ResetSpriteData();
           gMain.state++;
           break;
       case 3:
           LoadBgTiles(1, GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->tiles, 0x120, 0x1A2);
           gMain.state++;
           break;
       case 4:
           LoadPalette(sOptionMenuBg_Pal, BG_PLTT_ID(0), sizeof(sOptionMenuBg_Pal));
           LoadPalette(GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->pal, BG_PLTT_ID(7), PLTT_SIZE_4BPP);
           gMain.state++;
           break;
       case 5:
           LoadPalette(sOptionMenuText_Pal, BG_PLTT_ID(1), sizeof(sOptionMenuText_Pal));
           gMain.state++;
           break;
       case 6:
           PutWindowTilemap(WIN_HEADER);
           DrawHeaderText();
           gMain.state++;
           break;
       case 7:
           gMain.state++;
           break;
       case 8:
           PutWindowTilemap(WIN_OPTIONS);
           FillWindowPixelBuffer(WIN_OPTIONS, PIXEL_FILL(1));
           {  // draw names for the options that are initially scrolled into view
              u8 i;
              
              #if MENUITEM_COUNT <= MAX_MENU_ITEMS_VISIBLE_AT_ONCE
              for(i = 0; i < MENUITEM_COUNT; ++i)
              #else
              for(i = 0; i < MAX_MENU_ITEMS_VISIBLE_AT_ONCE; ++i)
              #endif
              {
                  DrawOptionName(&sOptions[i], i);
              }
           }
           CopyWindowToVram(WIN_OPTIONS, COPYWIN_FULL);
           gMain.state++;
       case 9:
           DrawBgWindowFrames();
           gMain.state++;
           break;
       case 10:
       {
           u8 taskId = CreateTask(Task_CGOptionMenuFadeIn, 0);

           gTasks[taskId].tMenuSelection = 0;
           gTasks[taskId].tMenuScrollOffset = 0;
           
           sTempOptions = gCustomGameOptions;

           {  // draw choices for the options that are initially scrolled into view
              u8 i;
              
              #if MENUITEM_COUNT <= MAX_MENU_ITEMS_VISIBLE_AT_ONCE
              for(i = 0; i < MENUITEM_COUNT; ++i)
              #else
              for(i = 0; i < MAX_MENU_ITEMS_VISIBLE_AT_ONCE; ++i)
              #endif
              {
                  DrawFullOptionRow(&sOptions[i], i);
              }
           }
           
           HighlightCGOptionMenuItem(gTasks[taskId].tMenuSelection - gTasks[taskId].tMenuScrollOffset);

           CopyWindowToVram(WIN_OPTIONS, COPYWIN_FULL);
           gMain.state++;
           break;
       }
       case 11:
           BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
           SetVBlankCallback(VBlankCB);
           SetMainCallback2(MainCB2);
           return;
   }
}

static void Task_CGOptionMenuFadeIn(u8 taskId)
{
    if (!gPaletteFade.active)
        gTasks[taskId].func = Task_CGOptionMenuProcessInput;
}

static void ScrollCGOptionMenuTo(u8 taskId, u8 to) {
   #if MENUITEM_COUNT > MAX_MENU_ITEMS_VISIBLE_AT_ONCE
      if (to > MENUITEM_COUNT - MAX_MENU_ITEMS_VISIBLE_AT_ONCE)
         to = MENUITEM_COUNT - MAX_MENU_ITEMS_VISIBLE_AT_ONCE;
   #else
      to = 0;
   #endif
   
   if (gTasks[taskId].tMenuScrollOffset == to)
      return;
      
   gTasks[taskId].tMenuScrollOffset = to;
   
   FillWindowPixelBuffer(WIN_OPTIONS, PIXEL_FILL(1));
   {
      u8 i;
      #if MENUITEM_COUNT > MAX_MENU_ITEMS_VISIBLE_AT_ONCE
      for(i = 0; i < MAX_MENU_ITEMS_VISIBLE_AT_ONCE; ++i)
      #else
      for(i = 0; i < MENUITEM_COUNT; ++i)
      #endif
      {
         u8 option_id = i + gTasks[taskId].tMenuScrollOffset;
         DrawFullOptionRow(&sOptions[option_id], i);
      }
   }
   
   // 100% necessary or we see a blank window when we scroll
   CopyWindowToVram(WIN_OPTIONS, COPYWIN_GFX);
}

static void ScrollCGOptionMenuBy(u8 taskId, s8 by) {
   u8 prior = gTasks[taskId].tMenuScrollOffset;
   if (by < 0) {
      if (prior - 1 < 0)
         return;
      if (prior + by < 0)
         by = -prior;
   } else {
      if (prior + 1 > MENUITEM_COUNT - MAX_MENU_ITEMS_VISIBLE_AT_ONCE)
         return;
      if (prior + by > MENUITEM_COUNT - MAX_MENU_ITEMS_VISIBLE_AT_ONCE)
         by = MENUITEM_COUNT - MAX_MENU_ITEMS_VISIBLE_AT_ONCE - prior;
   }
   ScrollCGOptionMenuTo(taskId, prior + by);
}

static void Task_CGOptionMenuProcessInput(u8 taskId) {
   if (JOY_NEW(A_BUTTON)) {
      if (gTasks[taskId].tMenuSelection == MENUITEM_CANCEL) {
         gTasks[taskId].func = Task_CGOptionMenuSave;
      }
      return;
   }
   if (JOY_NEW(B_BUTTON)) {
      gTasks[taskId].func = Task_CGOptionMenuSave;
      return;
   }
   
   // Up/Down: Move cursor, scrolling if necessary
   if (JOY_NEW(DPAD_UP)) {
      if (gTasks[taskId].tMenuSelection > 0) {
         gTasks[taskId].tMenuSelection--;
         if (gTasks[taskId].tMenuSelection - gTasks[taskId].tMenuScrollOffset < MENU_ITEM_HALFWAY_ROW) {
            ScrollCGOptionMenuBy(taskId, -1);
         }
      } else {
         gTasks[taskId].tMenuSelection = MENUITEM_CANCEL;
         ScrollCGOptionMenuTo(taskId, 255);
      }
      HighlightCGOptionMenuItem(gTasks[taskId].tMenuSelection - gTasks[taskId].tMenuScrollOffset);
      return;
   }
   if (JOY_NEW(DPAD_DOWN)) {
      if (gTasks[taskId].tMenuSelection < MENUITEM_CANCEL) {
         gTasks[taskId].tMenuSelection++;
         if (gTasks[taskId].tMenuSelection - gTasks[taskId].tMenuScrollOffset > MENU_ITEM_HALFWAY_ROW) {
            ScrollCGOptionMenuBy(taskId, 1);
         }
      } else {
         gTasks[taskId].tMenuSelection = 0;
         ScrollCGOptionMenuTo(taskId, 0);
      }
      HighlightCGOptionMenuItem(gTasks[taskId].tMenuSelection - gTasks[taskId].tMenuScrollOffset);
      return;
   }
   
   // Left/Right: Cycle option value
   {
      s8 by = JOY_NEW(DPAD_RIGHT) ? 1 : 0;
      if (!by) {
         by = JOY_NEW(DPAD_LEFT) ? -1 : 0;
      }
      if (by) {
         u8 option_id = gTasks[taskId].tMenuSelection;
         CycleOptionSelectedValue(&sOptions[option_id], by);
         DrawOptionRowValue(&sOptions[option_id], option_id - gTasks[taskId].tMenuScrollOffset);
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
    if (!gPaletteFade.active)
    {
        DestroyTask(taskId);
        FreeAllWindowBuffers();
        SetMainCallback2(gMain.savedCallback);
    }
}

static void HighlightCGOptionMenuItem(u8 index) {
   SetGpuReg(REG_OFFSET_WIN0H, WIN_RANGE(16, DISPLAY_WIDTH - 16));
   SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(index * 16 + 40, index * 16 + 56));
}

static void DrawFullOptionRow(const struct CGOptionMenuItem* item, u8 visibleRow) {
   DrawOptionName(item, visibleRow);
   DrawOptionRowValue(item, visibleRow);
}
static void DrawOptionName(const struct CGOptionMenuItem* item, u8 visibleRow) {
   AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, item->name, 8, (visibleRow * 16) + 1, TEXT_SKIP_DRAW, NULL);
}
static void DrawOptionRowValue(const struct CGOptionMenuItem* item, u8 visibleRow) {
   u8 selection;
   
   u8 text_width;
   u8 x_offset;
   u8 value_buffer[32];
   
   if (item->value_type == VALUE_TYPE_NONE || !item->value_strings)
      return;
   
   value_buffer[0] = EXT_CTRL_CODE_BEGIN;
   value_buffer[1] = EXT_CTRL_CODE_COLOR;
   value_buffer[2] = TEXT_COLOR_GREEN;
   value_buffer[3] = EXT_CTRL_CODE_BEGIN;
   value_buffer[4] = EXT_CTRL_CODE_SHADOW;
   value_buffer[5] = TEXT_COLOR_LIGHT_GREEN;
   
   selection = GetOptionSelectedValueIndex(item);
   
   {
      const u8* src;
      u8* dst;
      u16 i;
      
      src = item->value_strings[selection];
      dst = &value_buffer[6];
      for(i = 0; *src != EOS && i < ARRAY_COUNT(value_buffer) - 7; ++i) {
         dst[i] = *(src++);
      }
      dst[i] = EOS;
   }
   
   // left  edge is at 104px
   // right edge is at 198px
   // let's center the option value within the available space, for now
   // (it'll look neater once we display left/right arrows next to it)
   text_width = GetStringWidth(FONT_NORMAL, item->name, 0);
   x_offset   = (94 - text_width) / 2 + 104;
   
   AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, value_buffer, x_offset, (visibleRow * 16) + 1, TEXT_SKIP_DRAW, NULL);
}

static void DrawHeaderText(void) {
   FillWindowPixelBuffer(WIN_HEADER, PIXEL_FILL(1));
   AddTextPrinterParameterized(WIN_HEADER, FONT_NORMAL, gText_lu_CGO_menuTitle, 8, 1, TEXT_SKIP_DRAW, NULL);
   CopyWindowToVram(WIN_HEADER, COPYWIN_FULL);
}

#define TILE_TOP_CORNER_L 0x1A2
#define TILE_TOP_EDGE     0x1A3
#define TILE_TOP_CORNER_R 0x1A4
#define TILE_LEFT_EDGE    0x1A5
#define TILE_RIGHT_EDGE   0x1A7
#define TILE_BOT_CORNER_L 0x1A8
#define TILE_BOT_EDGE     0x1A9
#define TILE_BOT_CORNER_R 0x1AA

static void DrawBgWindowFrames(void)
{
    //                     bg, tile,              x, y, width, height, palNum
    // Draw title window frame
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_L,  1,  0,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_EDGE,      2,  0, 27,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_R, 28,  0,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_LEFT_EDGE,     1,  1,  1,  2,  7);
    FillBgTilemapBufferRect(1, TILE_RIGHT_EDGE,   28,  1,  1,  2,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_L,  1,  3,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_EDGE,      2,  3, 27,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_R, 28,  3,  1,  1,  7);

    // Draw options list window frame
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_L,  1,  4,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_EDGE,      2,  4, 26,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_R, 28,  4,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_LEFT_EDGE,     1,  5,  1, 18,  7);
    FillBgTilemapBufferRect(1, TILE_RIGHT_EDGE,   28,  5,  1, 18,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_L,  1, 19,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_EDGE,      2, 19, 26,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_R, 28, 19,  1,  1,  7);

    CopyBgTilemapBufferToVram(1);
}
