#include "global.h"
#include "trainer_pokemon_sprites.h"
#include "bg.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#include "data.h"
#include "decompress.h"
#include "event_data.h" // for checking badge count, etc.
//#include "field_effect.h"
#include "gpu_regs.h"
#include "graphics.h"
#include "international_string_util.h"
#include "link.h" // IsWirelessAdapterConnected; maybe more
#include "main.h"
#include "menu.h"
#include "list_menu.h" // AddScrollIndicatorArrowPair, RemoveScrollIndicatorArrowPair, Task_ScrollIndicatorArrowPairOnMainMenu; maybe more
#include "mystery_event_menu.h"
#include "option_menu.h"
#include "overworld.h"
#include "palette.h"
#include "pokedex.h"
#include "pokemon.h"
#include "rtc.h" // RtcGetErrorStatus, for the battery check; maybe more
#include "save.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "strings.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "title_screen.h"
#include "window.h"
#include "mystery_gift_menu.h"

#include "lu/string_wrap.h"
#include "new_game_introduction.h" // Prof. Birch intro moved to here

/*
 * Main menu state machine
 * -----------------------
 *
 * Entry point: CB2_InitMainMenu
 *
 * Note: States advance sequentially unless otherwise stated.
 *
 * CB2_InitMainMenu / CB2_ReinitMainMenu
 *  - Both of these states call InitMainMenu, which does all the work.
 *  - In the Reinit case, the init code will check if the user came from
 *    the options screen. If they did, then the options menu item is
 *    pre-selected.
 *
 * Task_MainMenuCheckSaveFile
 *  - Determines how many menu options to show based on whether
 *    the save file is Ok, empty, corrupted, etc.
 *  - If there was an error loading the save file, advance to
 *    Task_WaitForSaveFileErrorWindow.
 *  - If there were no errors, advance to Task_MainMenuCheckBattery.
 *  - Note that the check to enable Mystery Events would normally happen
 *    here, but this version of Emerald has them disabled.
 *
 * Task_WaitForSaveFileErrorWindow
 *  - Wait for the text to finish printing and then for the A button
 *    to be pressed.
 *
 * Task_MainMenuCheckBattery
 *  - If the battery is OK, advance to Task_DisplayMainMenu.
 *  - If the battery is dry, advance to Task_WaitForBatteryDryErrorWindow.
 *
 * Task_WaitForBatteryDryErrorWindow
 *  - Wait for the text to finish printing and then for the A button
 *    to be pressed.
 *
 * Task_DisplayMainWindow
 *  - Display the buttons to the user. If the menu is in HAS_MYSTERY_EVENTS
 *    mode, there are too many buttons for one screen and a scrollbar is added,
 *    and the scrollbar task is spawned (Task_ScrollIndicatorArrowPairOnMainMenu).
 *
 * Task_HighlightSelectedMainMenuItem
 *  - Update the UI to match the currently selected item.
 *
 * Task_HandleMainMenuInput
 *  - If A is pressed, advance to Task_HandleMainMenuAPressed.
 *  - If B is pressed, return to the title screen via CB2_InitTitleScreen.
 *  - If Up or Down is pressed, handle scrolling if there is a scroll bar, change
 *    the selection, then go back to Task_HighlightSelectedMainMenuItem.
 *
 * Task_HandleMainMenuAPressed
 *  - If the user selected New Game, advance to BeginNewGameIntro.
 *  - If the user selected Continue, advance to CB2_ContinueSavedGame.
 *  - If the user selected the Options menu, advance to CB2_InitOptionMenu.
 *  - If the user selected Mystery Gift, advance to CB2_InitMysteryGift. However,
 *    if the wireless adapter was removed, instead advance to
 *    Task_DisplayMainMenuInvalidActionError.
 *  - Code to start a Mystery Event is present here, but is unreachable in this
 *    version.
 *
 * Task_HandleMainMenuBPressed
 *  - Clean up the main menu and go back to CB2_InitTitleScreen.
 *
 * Task_DisplayMainMenuInvalidActionError
 *  - Print one of three different error messages, wait for the text to stop
 *    printing, and then wait for A or B to be pressed.
 * - Then advance to Task_HandleMainMenuBPressed.
 *
 * BeginNewGameIntro
 *  - Defined in `new_game_introduction.c`; was formerly here.
 *  - Reuses the current task (since again, it used to just be here).
 *  - Handles the entire Professor Birch intro.
 *  - Destroys the task and advances to CB2_NewGame when done.
 */

#define OPTION_MENU_FLAG (1 << 15)

// Static type declarations

// Static RAM declarations

static EWRAM_DATA u16 sCurrItemAndOptionMenuCheck = 0;

// Static ROM declarations

static u32 InitMainMenu(bool8);
static void Task_MainMenuCheckSaveFile(u8);
static void Task_MainMenuCheckBattery(u8);
static void Task_WaitForSaveFileErrorWindow(u8);
static void CreateMainMenuErrorWindow(const u8 *);
static void ClearMainMenuWindowTilemap(const struct WindowTemplate *);
static void Task_DisplayMainMenu(u8);
static void Task_WaitForBatteryDryErrorWindow(u8);
static void MainMenu_FormatSavegameText(void);
static void Task_HandleMainMenuInput(u8);
static void Task_HandleMainMenuAPressed(u8);
static void Task_HandleMainMenuBPressed(u8);

static void Task_DisplayMainMenuInvalidActionError(u8);

static void LoadMainMenuWindowFrameTiles(u8, u16);
static void DrawMainMenuWindowBorder(u8, u16, u8);
static void Task_HighlightSelectedMainMenuItem(u8);

static void MainMenu_FormatSavegamePlayer(void);
static void MainMenu_FormatSavegamePokedex(void);
static void MainMenu_FormatSavegameTime(void);
static void MainMenu_FormatSavegameBadges(void);

// .rodata

#define MENU_LEFT 2
#define MENU_TOP  1 // inner-top-edge of first menu item, measured in 4x4px tiles (so, 1 places the border flush with screen top)
//
#define MENU_WIDTH 26
//
#define MENU_LEFT_ERROR 2
#define MENU_TOP_ERROR 15
#define MENU_WIDTH_ERROR 26
#define MENU_HEIGHT_ERROR 4

#define BASE_BLOCK_OFFSET_PER_TEXT_LINE 0x34

#define BASE_BLOCK_OFFSET_NEW_GAME      0x01
#define BASE_BLOCK_OFFSET_OPTIONS       (BASE_BLOCK_OFFSET_NEW_GAME      + BASE_BLOCK_OFFSET_PER_TEXT_LINE)
#define BASE_BLOCK_OFFSET_CONTINUE      (BASE_BLOCK_OFFSET_OPTIONS       + BASE_BLOCK_OFFSET_PER_TEXT_LINE)
#define BASE_BLOCK_OFFSET_MYSTERY_GIFT  (BASE_BLOCK_OFFSET_CONTINUE      + (BASE_BLOCK_OFFSET_PER_TEXT_LINE * 3))
#define BASE_BLOCK_OFFSET_MYSTERY_EVENT (BASE_BLOCK_OFFSET_MYSTERY_GIFT  + BASE_BLOCK_OFFSET_PER_TEXT_LINE)
#define BASE_BLOCK_OFFSET_ERROR_MESSAGE (BASE_BLOCK_OFFSET_MYSTERY_EVENT + BASE_BLOCK_OFFSET_PER_TEXT_LINE)

#define MENUITEM_WINHEIGHT_SINGLE_ROW 2

#define MENU_SHADOW_PADDING 1

enum { // in display order
   WINDOW_MENUITEM_CONTINUE,
   WINDOW_MENUITEM_NEW_GAME,
   WINDOW_MENUITEM_MYSTERY_GIFT,
   WINDOW_MENUITEM_MYSTERY_EVENT,
   WINDOW_MENUITEM_OPTIONS,
   WINDOW_ERROR_MESSAGE,
   //
   WINDOW_COUNT,
   MENUITEM_COUNT = WINDOW_MENUITEM_OPTIONS + 1,
};
//
// Limits on menu item count:
//
//  - Every menu item is a window, and so is the error message. They're always drawn 
//    all at once, so we're limited to `WINDOWS_MAX - 1` menu items (i.e. 31).
//
//  - We currently map TextPrinter IDs directly to menu item IDs, so we're limited 
//    by the max number of text printers that can exist concurrently (minus one for 
//    the error message display). (NOTE: Does this limit actually apply for instant 
//    text? I thought it didn't...)
//
//     - That limit is also `WINDOWS_MAX`.
//
//  - We draw all menu items at once to a scrollable BG layer, so we're limited by 
//    its size, which is... 256x256px, I believe?
//

static u8 MenuItemHeight(u8 itemId) {
   if (itemId == WINDOW_MENUITEM_CONTINUE) {
      return (MENUITEM_WINHEIGHT_SINGLE_ROW * 3) + 2;
   }
   return MENUITEM_WINHEIGHT_SINGLE_ROW + 2;
}

#define MENU_WIN_HCOORDS WIN_RANGE(((MENU_LEFT - 1) * 8) + MENU_SHADOW_PADDING, (MENU_LEFT + MENU_WIDTH + 1) * 8 - MENU_SHADOW_PADDING)

// Use SetWindowAttribute(id, WINDOW_TILEMAP_TOP, top) to reorder the windows.
static const struct WindowTemplate sWindowTemplates_MainMenu[] = {
   [WINDOW_MENUITEM_CONTINUE] = {
      .bg          = 0,
      .tilemapLeft = MENU_LEFT,
      .tilemapTop  = MENU_TOP,
      .width       = MENU_WIDTH,
      .height      = (MENUITEM_WINHEIGHT_SINGLE_ROW * 3),
      .paletteNum  = 15,
      .baseBlock   = BASE_BLOCK_OFFSET_CONTINUE
   },
    [WINDOW_MENUITEM_NEW_GAME] = {
      .bg          = 0,
      .tilemapLeft = MENU_LEFT,
      .tilemapTop  = MENU_TOP,
      .width       = MENU_WIDTH,
      .height      = MENUITEM_WINHEIGHT_SINGLE_ROW,
      .paletteNum  = 15,
      .baseBlock   = BASE_BLOCK_OFFSET_NEW_GAME
   },
   [WINDOW_MENUITEM_MYSTERY_GIFT] = {
      .bg          = 0,
      .tilemapLeft = MENU_LEFT,
      .tilemapTop  = MENU_TOP,
      .width       = MENU_WIDTH,
      .height      = MENUITEM_WINHEIGHT_SINGLE_ROW,
      .paletteNum  = 15,
      .baseBlock   = BASE_BLOCK_OFFSET_MYSTERY_GIFT
   },
   [WINDOW_MENUITEM_MYSTERY_EVENT] = {
      .bg          = 0,
      .tilemapLeft = MENU_LEFT,
      .tilemapTop  = MENU_TOP,
      .width       = MENU_WIDTH,
      .height      = MENUITEM_WINHEIGHT_SINGLE_ROW,
      .paletteNum  = 15,
      .baseBlock   = BASE_BLOCK_OFFSET_MYSTERY_EVENT
   },
   [WINDOW_MENUITEM_OPTIONS] = {
      .bg          = 0,
      .tilemapLeft = MENU_LEFT,
      .tilemapTop  = MENU_TOP,
      .width       = MENU_WIDTH,
      .height      = MENUITEM_WINHEIGHT_SINGLE_ROW,
      .paletteNum  = 15,
      .baseBlock   = BASE_BLOCK_OFFSET_OPTIONS
   },
   [WINDOW_ERROR_MESSAGE] = {
      .bg          = 0,
      .tilemapLeft = MENU_LEFT_ERROR,
      .tilemapTop  = MENU_TOP_ERROR,
      .width       = MENU_WIDTH_ERROR,
      .height      = MENU_HEIGHT_ERROR,
      .paletteNum  = 15,
      .baseBlock   = BASE_BLOCK_OFFSET_ERROR_MESSAGE
   },
   DUMMY_WIN_TEMPLATE
};

static const u16 sMainMenuBgPal[] = INCBIN_U16("graphics/interface/main_menu_bg.gbapal");
static const u16 sMainMenuTextPal[] = INCBIN_U16("graphics/interface/main_menu_text.gbapal");

static const u8 sTextColor_Headers[] = {TEXT_DYNAMIC_COLOR_1, TEXT_DYNAMIC_COLOR_2, TEXT_DYNAMIC_COLOR_3};
static const u8 sTextColor_MenuInfo[] = {TEXT_DYNAMIC_COLOR_1, TEXT_COLOR_WHITE, TEXT_DYNAMIC_COLOR_3};

static const struct BgTemplate sMainMenuBgTemplates[] = {
   {
      .bg = 0,
      .charBaseIndex = 2,
      .mapBaseIndex = 30,
      .screenSize = 0,
      .paletteMode = 0,
      .priority = 0,
      .baseTile = 0
   },
   {
      .bg = 1,
      .charBaseIndex = 0,
      .mapBaseIndex = 7,
      .screenSize = 0,
      .paletteMode = 0,
      .priority = 3,
      .baseTile = 0
   }
};

static const struct ScrollArrowsTemplate sScrollArrowsTemplate_MainMenu = {2, 0x78, 8, 3, 0x78, 0x98, 3, 4, 1, 1, 0};

enum
{
    HAS_NO_SAVED_GAME,  //NEW GAME, OPTION
    HAS_SAVED_GAME,     //CONTINUE, NEW GAME, OPTION
    HAS_MYSTERY_GIFT,   //CONTINUE, NEW GAME, MYSTERY GIFT, OPTION
    HAS_MYSTERY_EVENTS, //CONTINUE, NEW GAME, MYSTERY GIFT, MYSTERY EVENTS, OPTION
};

enum
{
    ACTION_NEW_GAME,
    ACTION_CONTINUE,
    ACTION_OPTION,
    ACTION_MYSTERY_GIFT,
    ACTION_MYSTERY_EVENTS,
    ACTION_EREADER,
    ACTION_INVALID
};

#define MAIN_MENU_BORDER_TILE   0x1D5

static void CB2_MainMenu(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void VBlankCB_MainMenu(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

void CB2_InitMainMenu(void)
{
    InitMainMenu(FALSE);
}

void CB2_ReinitMainMenu(void)
{
    InitMainMenu(TRUE);
}

static u32 InitMainMenu(bool8 returningFromOptionsMenu)
{
    SetVBlankCallback(NULL);

    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    SetGpuReg(REG_OFFSET_BG2CNT, 0);
    SetGpuReg(REG_OFFSET_BG1CNT, 0);
    SetGpuReg(REG_OFFSET_BG0CNT, 0);
    SetGpuReg(REG_OFFSET_BG2HOFS, 0);
    SetGpuReg(REG_OFFSET_BG2VOFS, 0);
    SetGpuReg(REG_OFFSET_BG1HOFS, 0);
    SetGpuReg(REG_OFFSET_BG1VOFS, 0);
    SetGpuReg(REG_OFFSET_BG0HOFS, 0);
    SetGpuReg(REG_OFFSET_BG0VOFS, 0);

    DmaFill16(3, 0, (void *)VRAM, VRAM_SIZE);
    DmaFill32(3, 0, (void *)OAM, OAM_SIZE);
    DmaFill16(3, 0, (void *)(PLTT + 2), PLTT_SIZE - 2);

    ResetPaletteFade();
    LoadPalette(sMainMenuBgPal, BG_PLTT_ID(0), PLTT_SIZE_4BPP);
    LoadPalette(sMainMenuTextPal, BG_PLTT_ID(15), PLTT_SIZE_4BPP);
    ScanlineEffect_Stop();
    ResetTasks();
    ResetSpriteData();
    FreeAllSpritePalettes();
    if (returningFromOptionsMenu)
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0x10, 0, RGB_BLACK); // fade to black
    else
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0x10, 0, RGB_WHITEALPHA); // fade to white
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sMainMenuBgTemplates, ARRAY_COUNT(sMainMenuBgTemplates));
    ChangeBgX(0, 0, BG_COORD_SET);
    ChangeBgY(0, 0, BG_COORD_SET);
    ChangeBgX(1, 0, BG_COORD_SET);
    ChangeBgY(1, 0, BG_COORD_SET);
    InitWindows(sWindowTemplates_MainMenu);
    DeactivateAllTextPrinters();
    LoadMainMenuWindowFrameTiles(0, MAIN_MENU_BORDER_TILE);

    SetGpuReg(REG_OFFSET_WIN0H, 0);
    SetGpuReg(REG_OFFSET_WIN0V, 0);
    SetGpuReg(REG_OFFSET_WININ, 0);
    SetGpuReg(REG_OFFSET_WINOUT, 0);
    SetGpuReg(REG_OFFSET_BLDCNT, 0);
    SetGpuReg(REG_OFFSET_BLDALPHA, 0);
    SetGpuReg(REG_OFFSET_BLDY, 0);

    EnableInterrupts(1);
    SetVBlankCallback(VBlankCB_MainMenu);
    SetMainCallback2(CB2_MainMenu);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_WIN0_ON | DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
    ShowBg(0);
    HideBg(1);
    CreateTask(Task_MainMenuCheckSaveFile, 0);

    return 0;
}

#define tMenuType data[0]
#define tCurrItem data[1]
#define tItemCount data[12]
#define tScrollArrowTaskId data[13]
#define tIsScrolled data[14]
#define tWirelessAdapterConnected data[15]

#define tArrowTaskIsScrolled data[15]   // For scroll indicator arrow task

static u8 EnabledMenuItemsForMenuType(u16 menuType) {
   u8 menu_items_to_show = 0; // flags mask
   
   menu_items_to_show |= (1 << WINDOW_MENUITEM_NEW_GAME);
   menu_items_to_show |= (1 << WINDOW_MENUITEM_OPTIONS);
   
   if (menuType != HAS_NO_SAVED_GAME) {
      menu_items_to_show |= (1 << WINDOW_MENUITEM_CONTINUE);
      if (menuType == HAS_MYSTERY_GIFT) {
         menu_items_to_show |= (1 << WINDOW_MENUITEM_MYSTERY_GIFT);
      }
      if (menuType == HAS_MYSTERY_EVENTS) {
         menu_items_to_show |= (1 << WINDOW_MENUITEM_MYSTERY_GIFT) | (1 << WINDOW_MENUITEM_MYSTERY_EVENT);
      }
   }
   
   return menu_items_to_show;
}
static u8 MenuItemTopOffset(u8 itemId, u16 menuType) {
   u8 i;
   u8 item_top;
   u8 item_height;
   u8 menu_items_to_show   = 0; // flags mask
   u8 first_item_offscreen = 0xFF; // WINDOW_MENUITEM_*
   
   menu_items_to_show = EnabledMenuItemsForMenuType(menuType);
   
   item_top = 0;
   for(i = 0; i < itemId; ++i) {
      if ((menu_items_to_show & (1 << i)) == 0)
         continue;
      item_height = MenuItemHeight(i);
      if (first_item_offscreen == 0xFF && item_top + item_height > 20) {
         first_item_offscreen = i;
      }
      item_top += MenuItemHeight(i);
   }
   
   return item_top;
}
static u8 FirstIndexNeedingScrolling(u16 menuType) { // returns 0xFF if no scrolling
   u8 i;
   u8 item_top;
   u8 menu_items_to_show = 0; // flags mask
   
   menu_items_to_show = EnabledMenuItemsForMenuType(menuType);
   
   item_top = 0;
   for(i = 0; i < MENUITEM_COUNT; ++i) {
      if ((menu_items_to_show & (1 << i)) == 0)
         continue;
      item_top += MenuItemHeight(i);
      if (item_top > 20)
         return i;
   }
   
   return 0xFF;
}
static bool8 MenuNeedsScrolling(u16 menuType) {
   return FirstIndexNeedingScrolling(menuType) != 0xFF;
}

static u8 DisplayedMenuItemIndexToMenuItemId(u8 index, u16 menuType) {
   u8 i;
   u8 visible_items = EnabledMenuItemsForMenuType(menuType);
   
   for(i = 0; i < MENUITEM_COUNT; ++i) {
      if ((visible_items & (1 << i)) == 0)
         continue;
      if (index == 0) {
         return i;
      }
      --index;
   }
   return 0;
}
static u8 MenuItemIdToDisplayedMenuItemIndex(u8 id, u16 menuType) {
   u8 i;
   u8 count;
   u8 visible_items = EnabledMenuItemsForMenuType(menuType);
   
   count = 0;
   for(i = 0; i < id; ++i) {
      if ((visible_items & (1 << i)) == 0)
         continue;
      ++count;
   }
   return count;
}
static u8 ScrollYOffsetPixels(u8 taskId) {
   u8  i;
   u8  item_id;
   u8  item_top;
   u16 item_height;
   u8  visible_items;
   u16 pixels;
   
   if (sCurrItemAndOptionMenuCheck == 0) {
      return 0;
   }
   item_id = DisplayedMenuItemIndexToMenuItemId(sCurrItemAndOptionMenuCheck, gTasks[taskId].tMenuType);
   
   visible_items = EnabledMenuItemsForMenuType(gTasks[taskId].tMenuType);
   item_top      = 0;
   for(i = 0; i < item_id; ++i) {
      if ((visible_items & (1 << i)) == 0)
         continue;
      item_top += MenuItemHeight(i);
   }
   
   item_height = MenuItemHeight(item_id);
   
   if (item_top + item_height <= 20) {
      return 0;
   }
   pixels  = (item_top + item_height) * 8;
   pixels -= 160;
   return pixels;
}

static void UpdateScreenScroll(u8 taskId) {
   u8 scrollTask = gTasks[taskId].tScrollArrowTaskId;
   u8 offset     = ScrollYOffsetPixels(taskId);
   //
   ChangeBgY(0, offset << 8, BG_COORD_SET);
   ChangeBgY(1, offset << 8, BG_COORD_SET);
   
   if (offset > 0) {
      gTasks[taskId].tIsScrolled = TRUE;
   } else {
      gTasks[taskId].tIsScrolled = FALSE;
   }
   if (scrollTask != TASK_NONE) {
      gTasks[scrollTask].tArrowTaskIsScrolled = gTasks[taskId].tIsScrolled;
   }
}




static void Task_MainMenuCheckSaveFile(u8 taskId) {
    s16 *data = gTasks[taskId].data;

   if (gPaletteFade.active)
      return;
   
   SetGpuReg(REG_OFFSET_WIN0H, 0);
   SetGpuReg(REG_OFFSET_WIN0V, 0);
   SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_BG0 | WININ_WIN0_OBJ);
   SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_BG0 | WINOUT_WIN01_OBJ | WINOUT_WIN01_CLR);
   SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_EFFECT_DARKEN | BLDCNT_TGT1_BG0);
   SetGpuReg(REG_OFFSET_BLDALPHA, 0);
   SetGpuReg(REG_OFFSET_BLDY, 7);

   if (IsWirelessAdapterConnected())
      tWirelessAdapterConnected = TRUE;
   
   switch (gSaveFileStatus) {
      case SAVE_STATUS_OK:
          tMenuType = HAS_SAVED_GAME;
          if (IsMysteryGiftEnabled())
              tMenuType++;
          gTasks[taskId].func = Task_MainMenuCheckBattery;
          break;
      case SAVE_STATUS_CORRUPT:
          CreateMainMenuErrorWindow(gText_SaveFileErased);
          tMenuType = HAS_NO_SAVED_GAME;
          gTasks[taskId].func = Task_WaitForSaveFileErrorWindow;
          break;
      case SAVE_STATUS_ERROR:
          CreateMainMenuErrorWindow(gText_SaveFileCorrupted);
          gTasks[taskId].func = Task_WaitForSaveFileErrorWindow;
          tMenuType = HAS_SAVED_GAME;
          if (IsMysteryGiftEnabled() == TRUE)
              tMenuType++;
          break;
      case SAVE_STATUS_EMPTY:
      default:
          tMenuType = HAS_NO_SAVED_GAME;
          gTasks[taskId].func = Task_MainMenuCheckBattery;
          break;
      case SAVE_STATUS_NO_FLASH:
          CreateMainMenuErrorWindow(gJPText_No1MSubCircuit);
          gTasks[taskId].tMenuType = HAS_NO_SAVED_GAME;
          gTasks[taskId].func = Task_WaitForSaveFileErrorWindow;
          break;
   }
   {
      u8 i;
      u8 count       = 0;
      u8 enabled     = EnabledMenuItemsForMenuType(tMenuType);
      u8 options_row = 0;
      for(i = 0; i < MENUITEM_COUNT; ++i) {
         if (enabled & (1 << i)) {
            if (i == WINDOW_MENUITEM_OPTIONS)
               options_row = count;
            ++count;
         }
      }
      tItemCount = count;
      
      if (sCurrItemAndOptionMenuCheck & OPTION_MENU_FLAG) { // Are we returning from the Options menu?
         //
         // If so, highlight the OPTIONS menu item.
         //
         tCurrItem = sCurrItemAndOptionMenuCheck = options_row;
      }
   }
}

static void Task_WaitForSaveFileErrorWindow(u8 taskId) {
   RunTextPrinters();
   if (!IsTextPrinterActive(WINDOW_ERROR_MESSAGE) && (JOY_NEW(A_BUTTON))) {
      ClearWindowTilemap(WINDOW_ERROR_MESSAGE);
      ClearMainMenuWindowTilemap(&sWindowTemplates_MainMenu[WINDOW_ERROR_MESSAGE]);
      gTasks[taskId].func = Task_MainMenuCheckBattery;
   }
}

static void Task_MainMenuCheckBattery(u8 taskId) {
   if (!gPaletteFade.active) {
      SetGpuReg(REG_OFFSET_WIN0H, 0);
      SetGpuReg(REG_OFFSET_WIN0V, 0);
      SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_BG0 | WININ_WIN0_OBJ);
      SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_BG0 | WINOUT_WIN01_OBJ | WINOUT_WIN01_CLR);
      SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_EFFECT_DARKEN | BLDCNT_TGT1_BG0);
      SetGpuReg(REG_OFFSET_BLDALPHA, 0);
      SetGpuReg(REG_OFFSET_BLDY, 7);

      if (!(RtcGetErrorStatus() & RTC_ERR_FLAG_MASK)) {
         gTasks[taskId].func = Task_DisplayMainMenu;
      } else {
         CreateMainMenuErrorWindow(gText_BatteryRunDry);
         gTasks[taskId].func = Task_WaitForBatteryDryErrorWindow;
      }
   }
}

static void Task_WaitForBatteryDryErrorWindow(u8 taskId) {
   RunTextPrinters();
   if (!IsTextPrinterActive(WINDOW_ERROR_MESSAGE) && (JOY_NEW(A_BUTTON))) {
      ClearWindowTilemap(WINDOW_ERROR_MESSAGE);
      ClearMainMenuWindowTilemap(&sWindowTemplates_MainMenu[WINDOW_ERROR_MESSAGE]);
      gTasks[taskId].func = Task_DisplayMainMenu;
   }
}









static void Task_DisplayMainMenu(u8 taskId) {
   s16 *data = gTasks[taskId].data;
   u16 palette;
   
   if (gPaletteFade.active) {
      return;
   }

   SetGpuReg(REG_OFFSET_WIN0H, 0);
   SetGpuReg(REG_OFFSET_WIN0V, 0);
   SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_BG0 | WININ_WIN0_OBJ);
   SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_BG0 | WINOUT_WIN01_OBJ | WINOUT_WIN01_CLR);
   SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_EFFECT_DARKEN | BLDCNT_TGT1_BG0);
   SetGpuReg(REG_OFFSET_BLDALPHA, 0);
   SetGpuReg(REG_OFFSET_BLDY, 7);

   palette = RGB_BLACK;
   LoadPalette(&palette, BG_PLTT_ID(15) + 14, PLTT_SIZEOF(1));

   palette = RGB_WHITE;
   LoadPalette(&palette, BG_PLTT_ID(15) + 10, PLTT_SIZEOF(1));

   palette = RGB(12, 12, 12);
   LoadPalette(&palette, BG_PLTT_ID(15) + 11, PLTT_SIZEOF(1));

   palette = RGB(26, 26, 25);
   LoadPalette(&palette, BG_PLTT_ID(15) + 12, PLTT_SIZEOF(1));

   // Note: If there is no save file, the save block is zeroed out,
   // so the default gender is MALE.
   if (gSaveBlock2Ptr->playerGender == MALE) {
      palette = RGB(4, 16, 31);
      LoadPalette(&palette, BG_PLTT_ID(15) + 1, PLTT_SIZEOF(1));
   } else {
      palette = RGB(31, 3, 21);
      LoadPalette(&palette, BG_PLTT_ID(15) + 1, PLTT_SIZEOF(1));
   }
   
   {
      u8 i;
      u8 item_top;
      u8 item_height;
      u8 menu_items_to_show; // flags mask
      u8 first_item_offscreen = 0xFF; // WINDOW_MENUITEM_*
      
      menu_items_to_show = EnabledMenuItemsForMenuType(gTasks[taskId].tMenuType);
      
      item_top   = 0;
      tItemCount = 0;
      for(i = 0; i < MENUITEM_COUNT; ++i) {
         if ((menu_items_to_show & (1 << i)) == 0)
            continue;
         
         ++tItemCount;
         
         SetWindowAttribute(i, WINDOW_TILEMAP_TOP, item_top + MENU_TOP);
         item_height = MenuItemHeight(i);
         if (first_item_offscreen == 0xFF && item_top + item_height > 20) {
            first_item_offscreen = i;
         }
         
         FillWindowPixelBuffer(i, PIXEL_FILL(0xA));
         PutWindowTilemap(i);
         {
            const u8* text = gText_MainMenuNewGame;
            switch (i) {
               case WINDOW_MENUITEM_CONTINUE:
                  text = gText_MainMenuContinue;
                  MainMenu_FormatSavegameText(); // run text printer for "Continue" player name and stats
                  break;
               case WINDOW_MENUITEM_NEW_GAME:
                  text = gText_MainMenuNewGame;
                  break;
               case WINDOW_MENUITEM_MYSTERY_GIFT:
                  text = gText_MainMenuMysteryGift;
                  if (gTasks[taskId].tMenuType == HAS_MYSTERY_EVENTS) {
                     text = gText_MainMenuMysteryGift2;
                  }
                  break;
               case WINDOW_MENUITEM_MYSTERY_EVENT:
                  text = gText_MainMenuMysteryEvents;
                  break;
               case WINDOW_MENUITEM_OPTIONS:
                  text = gText_MainMenuOption;
                  break;
            }
            AddTextPrinterParameterized3(i, FONT_NORMAL, 0, 1, sTextColor_Headers, TEXT_SKIP_DRAW, text);
         }
         CopyWindowToVram(i, COPYWIN_GFX);
         DrawMainMenuWindowBorder(i, MAIN_MENU_BORDER_TILE, item_top + MENU_TOP);
         
         item_top += item_height;
      }
      
      if (item_top > 20) { // `item_top` is measured in 4x4px tiles
         tScrollArrowTaskId = AddScrollIndicatorArrowPair(&sScrollArrowsTemplate_MainMenu, &sCurrItemAndOptionMenuCheck);
         gTasks[tScrollArrowTaskId].func = Task_ScrollIndicatorArrowPairOnMainMenu;
         if (sCurrItemAndOptionMenuCheck >= first_item_offscreen) {
            UpdateScreenScroll(taskId);
         }
      } else {
         tScrollArrowTaskId = TASK_NONE;
      }
   }
   
   gTasks[taskId].func = Task_HighlightSelectedMainMenuItem;
}

static void Task_HighlightSelectedMainMenuItem(u8 taskId) {
   u8  item_id;
   u16 item_top;
   u16 item_bottom;
   
   SetGpuReg(REG_OFFSET_WIN0H, MENU_WIN_HCOORDS);
   
   item_id  = DisplayedMenuItemIndexToMenuItemId(gTasks[taskId].tCurrItem, gTasks[taskId].tMenuType);
   item_top = MenuItemTopOffset(item_id, gTasks[taskId].tMenuType);
   //
   item_top    = item_top * 8 + MENU_SHADOW_PADDING;
   item_bottom = item_top + (MenuItemHeight(item_id) * 8) - (MENU_SHADOW_PADDING * 2);
   
   if (gTasks[taskId].tIsScrolled) {
      u16 scroll = ScrollYOffsetPixels(taskId);
      //
      item_top    -= scroll;
      item_bottom -= scroll;
   }
   if (item_bottom >= 160) {
      item_bottom = 159;
   }
   SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(item_top, item_bottom));
   
   gTasks[taskId].func = Task_HandleMainMenuInput;
}

static bool8 HandleMainMenuInput(u8 taskId) {
   s16 *data = gTasks[taskId].data;

   if (JOY_NEW(A_BUTTON)) {
      PlaySE(SE_SELECT);
      IsWirelessAdapterConnected();   // why bother calling this here? debug? Task_HandleMainMenuAPressed will check too
      BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
      gTasks[taskId].func = Task_HandleMainMenuAPressed;
      return FALSE;
   }
   if (JOY_NEW(B_BUTTON)) {
      PlaySE(SE_SELECT);
      BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_WHITEALPHA);
      SetGpuReg(REG_OFFSET_WIN0H, WIN_RANGE(0, DISPLAY_WIDTH));
      SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(0, DISPLAY_HEIGHT));
      gTasks[taskId].func = Task_HandleMainMenuBPressed;
      return FALSE;
   }
   if ((JOY_NEW(DPAD_UP)) && tCurrItem > 0) {
      tCurrItem--;
      sCurrItemAndOptionMenuCheck = tCurrItem;
      UpdateScreenScroll(taskId);
      return TRUE;
   } else if ((JOY_NEW(DPAD_DOWN)) && tCurrItem < tItemCount - 1) {
      tCurrItem++;
      sCurrItemAndOptionMenuCheck = tCurrItem;
      UpdateScreenScroll(taskId);
      return TRUE;
   }
   return FALSE;
}

static void Task_HandleMainMenuInput(u8 taskId) {
   if (HandleMainMenuInput(taskId))
      gTasks[taskId].func = Task_HighlightSelectedMainMenuItem;
}

static void Task_HandleMainMenuAPressed(u8 taskId) {
   bool8 wirelessAdapterConnected;
   u8    action;
   u8    menu_item_id;
   
   if (gPaletteFade.active)
      return;

   if (MenuNeedsScrolling(gTasks[taskId].tMenuType) && gTasks[taskId].tScrollArrowTaskId != TASK_NONE)
      RemoveScrollIndicatorArrowPair(gTasks[taskId].tScrollArrowTaskId);
   
   {
      u8 i;
      for(i = 0; i < WINDOW_COUNT; ++i) {
         ClearStdWindowAndFrame(i, TRUE);
      }
   }
   menu_item_id = DisplayedMenuItemIndexToMenuItemId(gTasks[taskId].tCurrItem, gTasks[taskId].tMenuType);
   
   wirelessAdapterConnected = IsWirelessAdapterConnected();
   
   switch (menu_item_id) {
      case WINDOW_MENUITEM_CONTINUE:
         action = ACTION_CONTINUE;
         break;
      case WINDOW_MENUITEM_NEW_GAME:
      default:
         action = ACTION_NEW_GAME;
         break;
      case WINDOW_MENUITEM_MYSTERY_GIFT:
         if (gTasks[taskId].tMenuType == HAS_MYSTERY_EVENTS) {
            if (gTasks[taskId].tWirelessAdapterConnected) {
               action = ACTION_MYSTERY_GIFT;
               if (!wirelessAdapterConnected) {
                  action = ACTION_INVALID;
                  gTasks[taskId].tMenuType = HAS_NO_SAVED_GAME;
               }
            } else if (wirelessAdapterConnected) {
               action = ACTION_INVALID;
               gTasks[taskId].tMenuType = HAS_SAVED_GAME;
            } else {
               action = ACTION_EREADER;
            }
         } else {
            action = ACTION_MYSTERY_GIFT;
            if (!wirelessAdapterConnected) {
               action = ACTION_INVALID;
               gTasks[taskId].tMenuType = HAS_NO_SAVED_GAME;
            }
         }
         break;
      case WINDOW_MENUITEM_MYSTERY_EVENT:
         if (wirelessAdapterConnected) {
            action = ACTION_INVALID;
            gTasks[taskId].tMenuType = HAS_MYSTERY_GIFT;
         } else {
            action = ACTION_MYSTERY_EVENTS;
         }
         break;
      case WINDOW_MENUITEM_OPTIONS:
         action = ACTION_OPTION;
         break;
   }
   
   ChangeBgY(0, 0, BG_COORD_SET);
   ChangeBgY(1, 0, BG_COORD_SET);
     
   switch (action) {
      case ACTION_NEW_GAME:
      default:
         BeginNewGameIntro(taskId);
         break;
      case ACTION_CONTINUE:
         gPlttBufferUnfaded[0] = RGB_BLACK;
         gPlttBufferFaded[0]   = RGB_BLACK;
         SetMainCallback2(CB2_ContinueSavedGame);
         DestroyTask(taskId);
         break;
      case ACTION_OPTION:
         gMain.savedCallback = CB2_ReinitMainMenu;
         SetMainCallback2(CB2_InitOptionMenu);
         DestroyTask(taskId);
         break;
      case ACTION_MYSTERY_GIFT:
         SetMainCallback2(CB2_InitMysteryGift);
         DestroyTask(taskId);
         break;
      case ACTION_MYSTERY_EVENTS:
         SetMainCallback2(CB2_InitMysteryEventMenu);
         DestroyTask(taskId);
         break;
      case ACTION_EREADER:
         SetMainCallback2(CB2_InitEReader);
         DestroyTask(taskId);
          break;
      case ACTION_INVALID:
         gTasks[taskId].tCurrItem = 0;
         gTasks[taskId].func = Task_DisplayMainMenuInvalidActionError;
         gPlttBufferUnfaded[BG_PLTT_ID(15) + 1] = RGB_WHITE;
         gPlttBufferFaded[BG_PLTT_ID(15) + 1] = RGB_WHITE;
         SetGpuReg(REG_OFFSET_BG2HOFS, 0);
         SetGpuReg(REG_OFFSET_BG2VOFS, 0);
         SetGpuReg(REG_OFFSET_BG1HOFS, 0);
         SetGpuReg(REG_OFFSET_BG1VOFS, 0);
         SetGpuReg(REG_OFFSET_BG0HOFS, 0);
         SetGpuReg(REG_OFFSET_BG0VOFS, 0);
         BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
         return;
   }
   FreeAllWindowBuffers();
   if (action != ACTION_OPTION)
      sCurrItemAndOptionMenuCheck = 0;
   else
      sCurrItemAndOptionMenuCheck |= OPTION_MENU_FLAG;  // entering the options menu
}

static void Task_HandleMainMenuBPressed(u8 taskId) {
   if (!gPaletteFade.active) {
      if (MenuNeedsScrolling(gTasks[taskId].tMenuType) && gTasks[taskId].tScrollArrowTaskId != TASK_NONE)
         RemoveScrollIndicatorArrowPair(gTasks[taskId].tScrollArrowTaskId);
      sCurrItemAndOptionMenuCheck = 0;
      FreeAllWindowBuffers();
      SetMainCallback2(CB2_InitTitleScreen);
      DestroyTask(taskId);
   }
}

static void Task_DisplayMainMenuInvalidActionError(u8 taskId) {
   //
   // tCurrItem is co-opted as a task step/state value.
   // tMenuType is co-opted as the error message to display.
   //
   switch (gTasks[taskId].tCurrItem) {
      case 0:
         FillBgTilemapBufferRect_Palette0(0, 0, 0, 0, DISPLAY_TILE_WIDTH, DISPLAY_TILE_HEIGHT);
         switch (gTasks[taskId].tMenuType) {
            case 0:
               CreateMainMenuErrorWindow(gText_WirelessNotConnected);
               break;
            case 1:
               CreateMainMenuErrorWindow(gText_MysteryGiftCantUse);
               break;
            case 2:
               CreateMainMenuErrorWindow(gText_MysteryEventsCantUse);
               break;
         }
         gTasks[taskId].tCurrItem++;
         break;
      case 1:
         if (!gPaletteFade.active)
            gTasks[taskId].tCurrItem++;
         break;
      case 2:
         RunTextPrinters();
         if (!IsTextPrinterActive(WINDOW_ERROR_MESSAGE))
            gTasks[taskId].tCurrItem++;
         break;
      case 3:
         if (JOY_NEW(A_BUTTON | B_BUTTON)) {
            PlaySE(SE_SELECT);
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            gTasks[taskId].func = Task_HandleMainMenuBPressed;
         }
         break;
   }
}

#undef tMenuType
#undef tCurrItem
#undef tItemCount
#undef tScrollArrowTaskId
#undef tIsScrolled
#undef tWirelessAdapterConnected

#undef tArrowTaskIsScrolled

static void CreateMainMenuErrorWindow(const u8 *str) {
   FillWindowPixelBuffer(WINDOW_ERROR_MESSAGE, PIXEL_FILL(1));
   AddTextPrinterParameterized(WINDOW_ERROR_MESSAGE, FONT_NORMAL, str, 0, 1, 2, 0);
   PutWindowTilemap(WINDOW_ERROR_MESSAGE);
   CopyWindowToVram(WINDOW_ERROR_MESSAGE, COPYWIN_GFX);
   DrawMainMenuWindowBorder(WINDOW_ERROR_MESSAGE, MAIN_MENU_BORDER_TILE, sWindowTemplates_MainMenu[WINDOW_ERROR_MESSAGE].tilemapTop);
   SetGpuReg(REG_OFFSET_WIN0H, WIN_RANGE(  9, DISPLAY_WIDTH  - 9));
   SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(113, DISPLAY_HEIGHT - 1));
}

static void MainMenu_FormatSavegameText(void) {
   MainMenu_FormatSavegamePlayer();
   MainMenu_FormatSavegamePokedex();
   MainMenu_FormatSavegameTime();
   MainMenu_FormatSavegameBadges();
}

static void MainMenu_FormatSavegamePlayer(void) {
   StringExpandPlaceholders(gStringVar4, gText_ContinueMenuPlayer);
   AddTextPrinterParameterized3(WINDOW_MENUITEM_CONTINUE, FONT_NORMAL, 0, 17, sTextColor_MenuInfo, TEXT_SKIP_DRAW, gStringVar4);
   AddTextPrinterParameterized3(WINDOW_MENUITEM_CONTINUE, FONT_NORMAL, GetStringRightAlignXOffset(FONT_NORMAL, gSaveBlock2Ptr->playerName, 100), 17, sTextColor_MenuInfo, TEXT_SKIP_DRAW, gSaveBlock2Ptr->playerName);
}

static void MainMenu_FormatSavegameTime(void) {
   u8 str[0x20];
   u8 *ptr;

   StringExpandPlaceholders(gStringVar4, gText_ContinueMenuTime);
   AddTextPrinterParameterized3(WINDOW_MENUITEM_CONTINUE, FONT_NORMAL, 0x6C, 17, sTextColor_MenuInfo, TEXT_SKIP_DRAW, gStringVar4);
   ptr = ConvertIntToDecimalStringN(str, gSaveBlock2Ptr->playTimeHours, STR_CONV_MODE_LEFT_ALIGN, 3);
   *ptr = 0xF0;
   ConvertIntToDecimalStringN(ptr + 1, gSaveBlock2Ptr->playTimeMinutes, STR_CONV_MODE_LEADING_ZEROS, 2);
   AddTextPrinterParameterized3(WINDOW_MENUITEM_CONTINUE, FONT_NORMAL, GetStringRightAlignXOffset(FONT_NORMAL, str, 0xD0), 17, sTextColor_MenuInfo, TEXT_SKIP_DRAW, str);
}

static void MainMenu_FormatSavegamePokedex(void) {
   u8 str[0x20];
   u16 dexCount;

   if (FlagGet(FLAG_SYS_POKEDEX_GET) == TRUE) {
      if (IsNationalPokedexEnabled())
         dexCount = GetNationalPokedexCount(FLAG_GET_CAUGHT);
      else
         dexCount = GetHoennPokedexCount(FLAG_GET_CAUGHT);
        StringExpandPlaceholders(gStringVar4, gText_ContinueMenuPokedex);
      AddTextPrinterParameterized3(WINDOW_MENUITEM_CONTINUE, FONT_NORMAL, 0, 33, sTextColor_MenuInfo, TEXT_SKIP_DRAW, gStringVar4);
      ConvertIntToDecimalStringN(str, dexCount, STR_CONV_MODE_LEFT_ALIGN, 3);
      AddTextPrinterParameterized3(WINDOW_MENUITEM_CONTINUE, FONT_NORMAL, GetStringRightAlignXOffset(FONT_NORMAL, str, 100), 33, sTextColor_MenuInfo, TEXT_SKIP_DRAW, str);
   }
}

static void MainMenu_FormatSavegameBadges(void) {
   u8  str[0x20];
   u8  badgeCount = 0;
   u32 i;

   for (i = FLAG_BADGE01_GET; i < FLAG_BADGE01_GET + NUM_BADGES; i++) {
      if (FlagGet(i))
         badgeCount++;
   }
   StringExpandPlaceholders(gStringVar4, gText_ContinueMenuBadges);
   AddTextPrinterParameterized3(WINDOW_MENUITEM_CONTINUE, FONT_NORMAL, 0x6C, 33, sTextColor_MenuInfo, TEXT_SKIP_DRAW, gStringVar4);
   ConvertIntToDecimalStringN(str, badgeCount, STR_CONV_MODE_LEADING_ZEROS, 1);
   AddTextPrinterParameterized3(WINDOW_MENUITEM_CONTINUE, FONT_NORMAL, GetStringRightAlignXOffset(FONT_NORMAL, str, 0xD0), 33, sTextColor_MenuInfo, TEXT_SKIP_DRAW, str);
}

static void LoadMainMenuWindowFrameTiles(u8 bgId, u16 tileOffset) {
   LoadBgTiles(bgId, GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->tiles, 0x120, tileOffset);
   LoadPalette(GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->pal, BG_PLTT_ID(2), PLTT_SIZE_4BPP);
}

static void DrawMainMenuWindowBorder(u8 windowId, u16 baseTileNum, u8 tilemapTop) {
   const struct WindowTemplate* template = &sWindowTemplates_MainMenu[windowId];
   
   u16 r9 = 1 + baseTileNum;
   u16 r10 = 2 + baseTileNum;
   u16 sp18 = 3 + baseTileNum;
   u16 spC = 5 + baseTileNum;
   u16 sp10 = 6 + baseTileNum;
   u16 sp14 = 7 + baseTileNum;
   u16 r6 = 8 + baseTileNum;

   FillBgTilemapBufferRect(template->bg, baseTileNum, template->tilemapLeft - 1, tilemapTop - 1, 1, 1, 2);
   FillBgTilemapBufferRect(template->bg, r9, template->tilemapLeft, tilemapTop - 1, template->width, 1, 2);
   FillBgTilemapBufferRect(template->bg, r10, template->tilemapLeft + template->width, tilemapTop - 1, 1, 1, 2);
   FillBgTilemapBufferRect(template->bg, sp18, template->tilemapLeft - 1, tilemapTop, 1, template->height, 2);
   FillBgTilemapBufferRect(template->bg, spC, template->tilemapLeft + template->width, tilemapTop, 1, template->height, 2);
   FillBgTilemapBufferRect(template->bg, sp10, template->tilemapLeft - 1, tilemapTop + template->height, 1, 1, 2);
   FillBgTilemapBufferRect(template->bg, sp14, template->tilemapLeft, tilemapTop + template->height, template->width, 1, 2);
   FillBgTilemapBufferRect(template->bg, r6, template->tilemapLeft + template->width, tilemapTop + template->height, 1, 1, 2);
   CopyBgTilemapBufferToVram(template->bg);
}

static void ClearMainMenuWindowTilemap(const struct WindowTemplate *template) {
   FillBgTilemapBufferRect(template->bg, 0, template->tilemapLeft - 1, template->tilemapTop - 1, template->tilemapLeft + template->width + 1, template->tilemapTop + template->height + 1, 2);
   CopyBgTilemapBufferToVram(template->bg);
}

#undef tTimer
