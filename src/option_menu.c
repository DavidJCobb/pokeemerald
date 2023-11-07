#include "global.h"
#include "option_menu.h"
#include "bg.h"
#include "gpu_regs.h"
#include "international_string_util.h"
#include "main.h"
#include "menu.h"
#include "palette.h"
#include "scanline_effect.h"
#include "sprite.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "gba/m4a_internal.h"
#include "constants/rgb.h"

#define tMenuSelection data[0]
#define tMenuScrollOffset data[1]
#define tScrollArrowTaskId data[2]
#define tTextSpeed data[3]
#define tBattleSceneOff data[4]
#define tBattleStyle data[5]
#define tSound data[6]
#define tButtonMode data[7]
#define tWindowFrameType data[8]

#define tArrowTaskIsScrolled data[15]   // For scroll indicator arrow task

#include "lu/strings.h"
#include "lu/running_prefs.h"
#define SELECTED_OPTION_VALUE_FROM_TASK(task, n) (task).data[(n) + 3]
#define ON_SCREEN_OPTION_ROW_TO_Y_PIXELS(r) ((r) * 16)

#define MAX_MENU_ITEMS_VISIBLE_AT_ONCE 7

#define MENU_ITEM_HALFWAY_ROW (MAX_MENU_ITEMS_VISIBLE_AT_ONCE / 2)

#include "lu/custom_game_options_menu.h"

enum {
   WIN_HEADER,
   WIN_OPTIONS
};

static void Task_OptionMenuFadeIn(u8 taskId);
static void ScrollOptionMenuTo(u8 taskId, u8 to);
static void ScrollOptionMenuBy(u8 taskId, s8 by);
static void Task_OptionMenuProcessInput(u8 taskId);
static void Task_OptionMenuSave(u8 taskId);
static void Task_OptionMenuFadeOut(u8 taskId);
static void HighlightOptionMenuItem(u8 selection);
static u8 TextSpeed_ProcessInput(u8 selection);
static void TextSpeed_DrawChoices(u8 yOffsetPx, u8 selection);
static u8 BattleScene_ProcessInput(u8 selection);
static void BattleScene_DrawChoices(u8 yOffsetPx, u8 selection);
static u8 BattleStyle_ProcessInput(u8 selection);
static void BattleStyle_DrawChoices(u8 yOffsetPx, u8 selection);
static u8 Sound_ProcessInput(u8 selection);
static void Sound_DrawChoices(u8 yOffsetPx, u8 selection);
static u8 FrameType_ProcessInput(u8 selection);
static void FrameType_DrawChoices(u8 yOffsetPx, u8 selection);
static u8 ButtonMode_ProcessInput(u8 selection);
static void ButtonMode_DrawChoices(u8 yOffsetPx, u8 selection);
static u8 lu_Running_ProcessInput(u8 selection);
static void lu_Running_DrawChoices(u8 yOffsetPx, u8 selection);
static void DrawHeaderText(void); // draw menu title
static void DrawOptionMenuTexts(u8 rowOffset, bool8 updateVram); // draw option names, including "Cancel"; argument is number of rows by which we're scrolled down
static void DrawBgWindowFrames(void);

// Options' "process inputs" functions should set this to TRUE if the option's value 
// has changed. Updates to VRAM are done conditionally based on both changes to the 
// value (as detected from outside the "process inputs" function) and sArrowPressed.
EWRAM_DATA static bool8 sArrowPressed = FALSE;

static const u16 sOptionMenuText_Pal[] = INCBIN_U16("graphics/interface/option_menu_text.gbapal");
// note: this is only used in the Japanese release
static const u8 sEqualSignGfx[] = INCBIN_U8("graphics/interface/option_menu_equals_sign.4bpp");

#define MENUITEM_COUNT 8
#define MENUITEM_CANCEL (MENUITEM_COUNT - 1)

         #undef  MENUITEM_COUNT
         #define MENUITEM_COUNT 9
         #define MENUITEM_CUSTOM_GAME_OPTIONS (MENUITEM_COUNT - 2)

#define MENU_IS_SCROLLABLE 0
#if MENUITEM_COUNT > MAX_MENU_ITEMS_VISIBLE_AT_ONCE
   #undef MENU_IS_SCROLLABLE
   #define MENU_IS_SCROLLABLE 1
#endif

#define OPTION_VALUES_LEFT_EDGE  104
#define OPTION_VALUES_RIGHT_EDGE 198

//
// This array stores definitions for each menu item, including "Cancel." Be aware that 
// at present, we store the current state of each menu item in the menu's task data, 
// which has 16 fields -- two of which are currently used by the menu itself. This 
// means that we can define up to 14 menu items, including "Cancel," before we need 
// to either start storing that data somewhere else, or start storing it more cleverly 
// (e.g. the fields are u16s but the options hold u8s, so we could double storage with 
// some bitshifting shenanigans).
//
static const struct OptionMenuItem {
   // Option name
   const u8* name;
   
   // Handler for input, primarily left/right. Receives the option's current selected 
   // value as an argument. Should set `sArrowPressed` to `TRUE` if changing the value, 
   // and should return either the new current value (i.e. the argument if no changes).
   u8(*handle_input)(u8 selection);
   
   // Handler to update display of the option's value(s).
   // Arguments:
   //  - Y-position to draw text at, in pixels relative to the window's inner area
   //  - Selection
   void(*draw_choices)(u8 yOffsetPx, u8 selection);
} sOptions[MENUITEM_COUNT] = {
   {
      .name         = gText_TextSpeed,
      .handle_input = &TextSpeed_ProcessInput,
      .draw_choices = &TextSpeed_DrawChoices,
   },
   {
      .name         = gText_BattleScene,
      .handle_input = &BattleScene_ProcessInput,
      .draw_choices = &BattleScene_DrawChoices,
   },
   {
      .name         = gText_BattleStyle,
      .handle_input = &BattleStyle_ProcessInput,
      .draw_choices = &BattleStyle_DrawChoices,
   },
   {
      .name         = gText_Sound,
      .handle_input = &Sound_ProcessInput,
      .draw_choices = &Sound_DrawChoices,
   },
   {
      .name         = gText_ButtonMode,
      .handle_input = &ButtonMode_ProcessInput,
      .draw_choices = &ButtonMode_DrawChoices,
   },
   {
      .name         = gText_Frame,
      .handle_input = &FrameType_ProcessInput,
      .draw_choices = &FrameType_DrawChoices,
   },
   {
      .name         = gText_lu_option_running,
      .handle_input = &lu_Running_ProcessInput,
      .draw_choices = &lu_Running_DrawChoices,
   },
   #ifdef MENUITEM_CUSTOM_GAME_OPTIONS
   [MENUITEM_CUSTOM_GAME_OPTIONS] = {
      .name         = gText_lu_MainMenuCustomGameOptions,
      .handle_input = NULL,
      .draw_choices = NULL,
   },
   #endif
   [MENUITEM_CANCEL] = {
      .name         = gText_OptionMenuCancel,
      .handle_input = NULL,
      .draw_choices = NULL,
   },
};

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

static const struct ScrollArrowsTemplate sScrollArrowsTemplate_OptionsMenu = {
   .firstArrowType = SCROLL_ARROW_UP,
   .firstX = 120, // screen center
   .firstY = 40,
   .secondArrowType = SCROLL_ARROW_DOWN,
   .secondX = 120, // screen center
   .secondY = 0x98,
   .fullyUpThreshold = 0,
   .fullyDownThreshold = 0,
   .tileTag = 1,
   .palTag = 1,
   .palNum = 0
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

void CB2_InitOptionMenu(void)
{
    switch (gMain.state)
    {
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
        // WININ_WIN0_OBJ needed in next two calls in order for scroll indicators to be visible
        SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_BG0 | WININ_WIN0_OBJ);
        SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_BG0 | WINOUT_WIN01_BG1 | WINOUT_WIN01_CLR | WININ_WIN0_OBJ);
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
        DrawOptionMenuTexts(0, TRUE);
        gMain.state++;
    case 9:
        DrawBgWindowFrames();
        gMain.state++;
        break;
    case 10:
    {
        u8 taskId = CreateTask(Task_OptionMenuFadeIn, 0);

        gTasks[taskId].tMenuSelection = 0;
        gTasks[taskId].tMenuScrollOffset = 0;
        
        gTasks[taskId].tTextSpeed = gSaveBlock2Ptr->optionsTextSpeed;
        gTasks[taskId].tBattleSceneOff = gSaveBlock2Ptr->optionsBattleSceneOff;
        gTasks[taskId].tBattleStyle = gSaveBlock2Ptr->optionsBattleStyle;
        gTasks[taskId].tSound = gSaveBlock2Ptr->optionsSound;
        gTasks[taskId].tButtonMode = gSaveBlock2Ptr->optionsButtonMode;
        gTasks[taskId].tWindowFrameType = gSaveBlock2Ptr->optionsWindowFrameType;
        SELECTED_OPTION_VALUE_FROM_TASK(gTasks[taskId], 6) = gSaveBlock2Ptr->optionsRunningToggle;

        {  // draw choices for the options that are initially scrolled into view
           u8 i;
           
           #if MENU_IS_SCROLLABLE
           for(i = 0; i < MAX_MENU_ITEMS_VISIBLE_AT_ONCE; ++i)
           #else
           for(i = 0; i < MENUITEM_COUNT; ++i)
           #endif
           {
              if (sOptions[i].draw_choices) {
                 sOptions[i].draw_choices(
                    ON_SCREEN_OPTION_ROW_TO_Y_PIXELS(i),
                    SELECTED_OPTION_VALUE_FROM_TASK(gTasks[taskId], i)
                 );
              }
           }
        }
        
        HighlightOptionMenuItem(gTasks[taskId].tMenuSelection - gTasks[taskId].tMenuScrollOffset);
        
        #if MENU_IS_SCROLLABLE
        {
            struct ScrollArrowsTemplate tmpl = sScrollArrowsTemplate_OptionsMenu;
            tmpl.fullyDownThreshold = MENUITEM_COUNT - MAX_MENU_ITEMS_VISIBLE_AT_ONCE;
            //
            gTasks[taskId].tScrollArrowTaskId = AddScrollIndicatorArrowPair(
                &tmpl,
                &gTasks[taskId].tMenuScrollOffset
            );
        }
        #endif

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

static void Task_OptionMenuFadeIn(u8 taskId)
{
    if (!gPaletteFade.active)
        gTasks[taskId].func = Task_OptionMenuProcessInput;
}

static void ScrollOptionMenuTo(u8 taskId, u8 to) {
   #if MENU_IS_SCROLLABLE
      if (to > MENUITEM_COUNT - MAX_MENU_ITEMS_VISIBLE_AT_ONCE)
         to = MENUITEM_COUNT - MAX_MENU_ITEMS_VISIBLE_AT_ONCE;
   #else
      to = 0;
   #endif
   
   if (gTasks[taskId].tMenuScrollOffset == to)
      return;
      
   gTasks[taskId].tMenuScrollOffset = to;
   
   #if MENU_IS_SCROLLABLE
      gTasks[gTasks[taskId].tScrollArrowTaskId].tArrowTaskIsScrolled = (to != 0);
   #endif
   
   FillWindowPixelBuffer(WIN_OPTIONS, PIXEL_FILL(1));
   DrawOptionMenuTexts(gTasks[taskId].tMenuScrollOffset, FALSE);
   {
      u8 i;
      #if MENU_IS_SCROLLABLE
      for(i = 0; i < MAX_MENU_ITEMS_VISIBLE_AT_ONCE; ++i) {
      #else
      for(i = 0; i < MENUITEM_COUNT; ++i) {
      #endif
         u8 option_id = i + gTasks[taskId].tMenuScrollOffset;
      
         // NOTE: Because we use `TEXT_SKIP_DRAW`, text is drawn to VRAM immediately and no persistent 
         // text printer is needed. This means that we can use this function in a fire-and-forget way 
         // without having to worry about too many text printers being active.
         AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, sOptions[option_id].name, 8, (i * 16) + 1, TEXT_SKIP_DRAW, NULL);
         
         // The "Cancel" menu item does not define a "draw choices" handler.
         if (sOptions[option_id].draw_choices) {
            (sOptions[option_id].draw_choices)(
               ON_SCREEN_OPTION_ROW_TO_Y_PIXELS(i),
               SELECTED_OPTION_VALUE_FROM_TASK(gTasks[taskId], option_id)
            );
         }
      }
   }
   
   // 100% necessary or we see a blank window when we scroll
   CopyWindowToVram(WIN_OPTIONS, COPYWIN_GFX);
}

static void ScrollOptionMenuBy(u8 taskId, s8 by) {
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
   ScrollOptionMenuTo(taskId, prior + by);
}

static void Task_OptionMenuProcessInput(u8 taskId) {
   if (JOY_NEW(A_BUTTON)) {
      if (gTasks[taskId].tMenuSelection == MENUITEM_CANCEL) {
         gTasks[taskId].func = Task_OptionMenuSave;
      }
      #ifdef MENUITEM_CUSTOM_GAME_OPTIONS
      if (gTasks[taskId].tMenuSelection == MENUITEM_CUSTOM_GAME_OPTIONS) {
         SetMainCallback2(CB2_InitCustomGameOptionMenu);
         #if MENU_IS_SCROLLABLE
            RemoveScrollIndicatorArrowPair(gTasks[taskId].tScrollArrowTaskId);
         #endif
         DestroyTask(taskId);
         FreeAllWindowBuffers();
         return;
      }
      #endif
   }
    else if (JOY_NEW(B_BUTTON))
    {
        gTasks[taskId].func = Task_OptionMenuSave;
    }
    else if (JOY_NEW(DPAD_UP))
    {
        if (gTasks[taskId].tMenuSelection > 0) {
            gTasks[taskId].tMenuSelection--;
            if (gTasks[taskId].tMenuSelection - gTasks[taskId].tMenuScrollOffset < MENU_ITEM_HALFWAY_ROW) {
               ScrollOptionMenuBy(taskId, -1);
            }
        } else {
            gTasks[taskId].tMenuSelection = MENUITEM_CANCEL;
            ScrollOptionMenuTo(taskId, 255);
        }
        HighlightOptionMenuItem(gTasks[taskId].tMenuSelection - gTasks[taskId].tMenuScrollOffset);
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        if (gTasks[taskId].tMenuSelection < MENUITEM_CANCEL) {
            gTasks[taskId].tMenuSelection++;
            if (gTasks[taskId].tMenuSelection - gTasks[taskId].tMenuScrollOffset > MENU_ITEM_HALFWAY_ROW) {
               ScrollOptionMenuBy(taskId, 1);
            }
        } else {
            gTasks[taskId].tMenuSelection = 0;
            ScrollOptionMenuTo(taskId, 0);
        }
        HighlightOptionMenuItem(gTasks[taskId].tMenuSelection - gTasks[taskId].tMenuScrollOffset);
    }
    else
    {
        u8 option_id = gTasks[taskId].tMenuSelection;
        u8 priorValue;
        u8 afterValue;
        
         #ifdef MENUITEM_CUSTOM_GAME_OPTIONS
         if (option_id == MENUITEM_CUSTOM_GAME_OPTIONS) {
            return;
         }
         #endif
        if (option_id >= MENUITEM_CANCEL) {
           return;
        }
        priorValue = (u8) SELECTED_OPTION_VALUE_FROM_TASK(gTasks[taskId], option_id);
        afterValue = (sOptions[option_id].handle_input)(priorValue);
        SELECTED_OPTION_VALUE_FROM_TASK(gTasks[taskId], option_id) = afterValue;
        
        if (priorValue != afterValue) {
           (sOptions[option_id].draw_choices)(
              ON_SCREEN_OPTION_ROW_TO_Y_PIXELS(option_id - gTasks[taskId].tMenuScrollOffset),
              afterValue
           );
        }

        if (sArrowPressed)
        {
            sArrowPressed = FALSE;
            CopyWindowToVram(WIN_OPTIONS, COPYWIN_GFX);
        }
    }
}

static void Task_OptionMenuSave(u8 taskId)
{
    gSaveBlock2Ptr->optionsTextSpeed = gTasks[taskId].tTextSpeed;
    gSaveBlock2Ptr->optionsBattleSceneOff = gTasks[taskId].tBattleSceneOff;
    gSaveBlock2Ptr->optionsBattleStyle = gTasks[taskId].tBattleStyle;
    gSaveBlock2Ptr->optionsSound = gTasks[taskId].tSound;
    gSaveBlock2Ptr->optionsButtonMode = gTasks[taskId].tButtonMode;
    gSaveBlock2Ptr->optionsWindowFrameType = gTasks[taskId].tWindowFrameType;
    lu_SetOverworldRunOption(
       SELECTED_OPTION_VALUE_FROM_TASK(gTasks[taskId], 6)
    );

    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    gTasks[taskId].func = Task_OptionMenuFadeOut;
}

static void Task_OptionMenuFadeOut(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        #if MENU_IS_SCROLLABLE
            RemoveScrollIndicatorArrowPair(gTasks[taskId].tScrollArrowTaskId);
        #endif
        DestroyTask(taskId);
        FreeAllWindowBuffers();
        SetMainCallback2(gMain.savedCallback);
    }
}

static void HighlightOptionMenuItem(u8 index)
{
    SetGpuReg(REG_OFFSET_WIN0H, WIN_RANGE(16, DISPLAY_WIDTH - 16));
    SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(index * 16 + 40, index * 16 + 56));
}

static void DrawOptionMenuChoice(const u8 *text, u8 x, u8 y, u8 style)
{
    u8 dst[16];
    u16 i;

    for (i = 0; *text != EOS && i < ARRAY_COUNT(dst) - 1; i++)
        dst[i] = *(text++);

    // Option value strings are meant to begin with format codes. This directly overwrites 
    // parameter bytes within these format codes, to distinguish the selected option value 
    // from the other values.
    if (style != 0)
    {
        dst[2] = TEXT_COLOR_RED;
        dst[5] = TEXT_COLOR_LIGHT_RED;
    }

    dst[i] = EOS;
    AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, dst, x, y + 1, TEXT_SKIP_DRAW, NULL);
}

static u8 TextSpeed_ProcessInput(u8 selection)
{
    if (JOY_NEW(DPAD_RIGHT))
    {
        if (selection <= 1)
            selection++;
        else
            selection = 0;

        sArrowPressed = TRUE;
    }
    if (JOY_NEW(DPAD_LEFT))
    {
        if (selection != 0)
            selection--;
        else
            selection = 2;

        sArrowPressed = TRUE;
    }
    return selection;
}

static void TextSpeed_DrawChoices(u8 yOffsetPx, u8 selection)
{
    u8 styles[3];
    s32 widthSlow, widthMid, widthFast, xMid;

    styles[0] = 0;
    styles[1] = 0;
    styles[2] = 0;
    styles[selection] = 1;

    DrawOptionMenuChoice(gText_TextSpeedSlow, OPTION_VALUES_LEFT_EDGE, yOffsetPx, styles[0]);

    widthSlow = GetStringWidth(FONT_NORMAL, gText_TextSpeedSlow, 0);
    widthMid = GetStringWidth(FONT_NORMAL, gText_TextSpeedMid, 0);
    widthFast = GetStringWidth(FONT_NORMAL, gText_TextSpeedFast, 0);

    widthMid -= 94;
    xMid = (widthSlow - widthMid - widthFast) / 2 + OPTION_VALUES_LEFT_EDGE;
    DrawOptionMenuChoice(gText_TextSpeedMid, xMid, yOffsetPx, styles[1]);

    DrawOptionMenuChoice(gText_TextSpeedFast, GetStringRightAlignXOffset(FONT_NORMAL, gText_TextSpeedFast, OPTION_VALUES_RIGHT_EDGE), yOffsetPx, styles[2]);
}

static u8 BattleScene_ProcessInput(u8 selection)
{
    if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT))
    {
        selection ^= 1;
        sArrowPressed = TRUE;
    }

    return selection;
}

static void BattleScene_DrawChoices(u8 yOffsetPx, u8 selection)
{
    u8 styles[2];

    styles[0] = 0;
    styles[1] = 0;
    styles[selection] = 1;

    DrawOptionMenuChoice(gText_BattleSceneOn, OPTION_VALUES_LEFT_EDGE, yOffsetPx, styles[0]);
    DrawOptionMenuChoice(gText_BattleSceneOff, GetStringRightAlignXOffset(FONT_NORMAL, gText_BattleSceneOff, OPTION_VALUES_RIGHT_EDGE), yOffsetPx, styles[1]);
}

static u8 BattleStyle_ProcessInput(u8 selection)
{
    if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT))
    {
        selection ^= 1;
        sArrowPressed = TRUE;
    }

    return selection;
}

static void BattleStyle_DrawChoices(u8 yOffsetPx, u8 selection)
{
    u8 styles[2];

    styles[0] = 0;
    styles[1] = 0;
    styles[selection] = 1;

    DrawOptionMenuChoice(gText_BattleStyleShift, OPTION_VALUES_LEFT_EDGE, yOffsetPx, styles[0]);
    DrawOptionMenuChoice(gText_BattleStyleSet, GetStringRightAlignXOffset(FONT_NORMAL, gText_BattleStyleSet, OPTION_VALUES_RIGHT_EDGE), yOffsetPx, styles[1]);
}

static u8 Sound_ProcessInput(u8 selection)
{
    if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT))
    {
        selection ^= 1;
        SetPokemonCryStereo(selection);
        sArrowPressed = TRUE;
    }

    return selection;
}

static void Sound_DrawChoices(u8 yOffsetPx, u8 selection)
{
    u8 styles[2];

    styles[0] = 0;
    styles[1] = 0;
    styles[selection] = 1;

    DrawOptionMenuChoice(gText_SoundMono, OPTION_VALUES_LEFT_EDGE, yOffsetPx, styles[0]);
    DrawOptionMenuChoice(gText_SoundStereo, GetStringRightAlignXOffset(FONT_NORMAL, gText_SoundStereo, OPTION_VALUES_RIGHT_EDGE), yOffsetPx, styles[1]);
}

static u8 FrameType_ProcessInput(u8 selection)
{
    if (JOY_NEW(DPAD_RIGHT))
    {
        if (selection < WINDOW_FRAMES_COUNT - 1)
            selection++;
        else
            selection = 0;

        LoadBgTiles(1, GetWindowFrameTilesPal(selection)->tiles, 0x120, 0x1A2);
        LoadPalette(GetWindowFrameTilesPal(selection)->pal, BG_PLTT_ID(7), PLTT_SIZE_4BPP);
        sArrowPressed = TRUE;
    }
    if (JOY_NEW(DPAD_LEFT))
    {
        if (selection != 0)
            selection--;
        else
            selection = WINDOW_FRAMES_COUNT - 1;

        LoadBgTiles(1, GetWindowFrameTilesPal(selection)->tiles, 0x120, 0x1A2);
        LoadPalette(GetWindowFrameTilesPal(selection)->pal, BG_PLTT_ID(7), PLTT_SIZE_4BPP);
        sArrowPressed = TRUE;
    }
    return selection;
}

static void FrameType_DrawChoices(u8 yOffsetPx, u8 selection)
{
    u8 text[16];
    u8 n = selection + 1;
    u16 i;

    for (i = 0; gText_FrameTypeNumber[i] != EOS && i <= 5; i++)
        text[i] = gText_FrameTypeNumber[i];

    // Convert a number to decimal string
    if (n / 10 != 0)
    {
        text[i] = n / 10 + CHAR_0;
        i++;
        text[i] = n % 10 + CHAR_0;
        i++;
    }
    else
    {
        text[i] = n % 10 + CHAR_0;
        i++;
        text[i] = CHAR_SPACER;
        i++;
    }

    text[i] = EOS;

    DrawOptionMenuChoice(gText_FrameType, OPTION_VALUES_LEFT_EDGE, yOffsetPx, 0);
    DrawOptionMenuChoice(text, 128, yOffsetPx, 1);
}

static u8 ButtonMode_ProcessInput(u8 selection)
{
    if (JOY_NEW(DPAD_RIGHT))
    {
        if (selection <= 1)
            selection++;
        else
            selection = 0;

        sArrowPressed = TRUE;
    }
    if (JOY_NEW(DPAD_LEFT))
    {
        if (selection != 0)
            selection--;
        else
            selection = 2;

        sArrowPressed = TRUE;
    }
    return selection;
}

static void ButtonMode_DrawChoices(u8 yOffsetPx, u8 selection)
{
    s32 widthNormal, widthLR, widthLA, xLR;
    u8 styles[3];

    styles[0] = 0;
    styles[1] = 0;
    styles[2] = 0;
    styles[selection] = 1;

    DrawOptionMenuChoice(gText_ButtonTypeNormal, OPTION_VALUES_LEFT_EDGE, yOffsetPx, styles[0]);

    widthNormal = GetStringWidth(FONT_NORMAL, gText_ButtonTypeNormal, 0);
    widthLR = GetStringWidth(FONT_NORMAL, gText_ButtonTypeLR, 0);
    widthLA = GetStringWidth(FONT_NORMAL, gText_ButtonTypeLEqualsA, 0);

    widthLR -= 94;
    xLR = (widthNormal - widthLR - widthLA) / 2 + OPTION_VALUES_LEFT_EDGE;
    DrawOptionMenuChoice(gText_ButtonTypeLR, xLR, yOffsetPx, styles[1]);

    DrawOptionMenuChoice(gText_ButtonTypeLEqualsA, GetStringRightAlignXOffset(FONT_NORMAL, gText_ButtonTypeLEqualsA, OPTION_VALUES_RIGHT_EDGE), yOffsetPx, styles[2]);
}

static u8 lu_Running_ProcessInput(u8 selection) {
    // Since there's only two options, it's trivial to make both left and right toggle/cycle 
    // between them.
    if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT)) {
        selection ^= 1;
        sArrowPressed = TRUE;
    }

    return selection;
}
static void lu_Running_DrawChoices(u8 yOffsetPx, u8 selection) {
    u8 styles[2];

    styles[0] = 0;
    styles[1] = 0;
    styles[selection] = 1;

    DrawOptionMenuChoice(gText_lu_option_running_vanilla, OPTION_VALUES_LEFT_EDGE, yOffsetPx, styles[0]);
    DrawOptionMenuChoice(gText_lu_option_running_toggle, GetStringRightAlignXOffset(FONT_NORMAL, gText_lu_option_running_toggle, OPTION_VALUES_RIGHT_EDGE), yOffsetPx, styles[1]);
}

static void DrawHeaderText(void)
{
    FillWindowPixelBuffer(WIN_HEADER, PIXEL_FILL(1));
    AddTextPrinterParameterized(WIN_HEADER, FONT_NORMAL, gText_Option, 8, 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(WIN_HEADER, COPYWIN_FULL);
}

static void DrawOptionMenuTexts(u8 rowOffset, bool8 updateVram) {
    u8 i;

    FillWindowPixelBuffer(WIN_OPTIONS, PIXEL_FILL(1));
    for (i = 0; i < MAX_MENU_ITEMS_VISIBLE_AT_ONCE; i++) {
       u8 option_id = i + rowOffset;
       
        // NOTE: Because we use `TEXT_SKIP_DRAW`, text is drawn to VRAM immediately and no persistent 
        // text printer is needed. This means that we can use this function in a fire-and-forget way 
        // without having to worry about too many text printers being active.
        AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, sOptions[option_id].name, 8, (i * 16) + 1, TEXT_SKIP_DRAW, NULL);
    }
    if (updateVram)
      CopyWindowToVram(WIN_OPTIONS, COPYWIN_FULL);
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
