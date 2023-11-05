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

#include "characters.h"
#include "data/text/species_names.h"
#include "lu/strings.h"

enum {
   WIN_HEADER,
   WIN_OPTIONS,
   WIN_KEYBINDS_STRIP,
};

#define MAX_MENU_ITEMS_VISIBLE_AT_ONCE 7
//
#define MENU_ITEM_HALFWAY_ROW (MAX_MENU_ITEMS_VISIBLE_AT_ONCE / 2)

EWRAM_DATA static struct CustomGameOptions sTempOptions;

enum { // CGOptionMenuItem::value_type
   VALUE_TYPE_NONE, // e.g. "Cancel"
   VALUE_TYPE_U8,
   VALUE_TYPE_U16,
   VALUE_TYPE_BOOL8,
   
   VALUE_TYPE_POKEMON_SPECIES,
};

enum { // CGOptionMenuItem::flags
   MENUITEM_FLAG_END_OF_LIST_SENTINEL,
   MENUITEM_FLAG_IS_ENUM,
   MENUITEM_FLAG_IS_SUBMENU,
   MENUITEM_FLAG_0_MEANS_DISABLED, // Display zero as "Disabled" instead of "0"
   MENUITEM_FLAG_0_MEANS_DEFAULT,  // Display zero as "Default" instead of "0"
   MENUITEM_FLAG_POKEMON_SPECIES_ALLOW_0, // Allow zero as a Pokemon species number (displays as "None" by default)
};

struct CGOptionMenuItem {
   const u8* name; // == NULL for (sub)menu end sentinel
   const u8* help_string;
   
   u8 flags;
   u8 value_type;
   union {
      struct {
         const u8** name_strings;
         const u8** help_strings;
         u8 count;
      } named; // used if MENUITEM_FLAG_IS_ENUM is set
      struct {
         s16 min;
         s16 max;
      } integral; // used if MENUITEM_FLAG_IS_ENUM is not set and the `value_type` is marked as u8 or u16
   } values;
   union {
      bool8* as_bool8;
      u8*    as_u8;
      u16*   as_u16;
      const struct CGOptionMenuItem* submenu;
   } target;
};

static bool8 MenuItemIsListTerminator(const struct CGOptionMenuItem* item) {
   if (item->name == NULL)
      return TRUE;
   if (item->flags & (1 << MENUITEM_FLAG_END_OF_LIST_SENTINEL))
      return TRUE;
   return FALSE;
}
static u8 GetMenuItemListCount(const struct CGOptionMenuItem* items) {
   u8 i;
   for(i = 0; !MenuItemIsListTerminator(&items[i]); ++i) {
      ;
   }
   return i;
}

static const struct CGOptionMenuItem sEndOfListSentinel = {
   .name        = NULL,
   .help_string = NULL,
   .flags       = (1 << MENUITEM_FLAG_END_OF_LIST_SENTINEL),
   .value_type  = VALUE_TYPE_NONE,
   .target      = NULL,
};

static const u16 GetOptionValue(const struct CGOptionMenuItem* item) {
   switch (item->value_type) {
      case VALUE_TYPE_BOOL8:
         return *item->target.as_bool8 ? 1 : 0;
      case VALUE_TYPE_U8:
         return *item->target.as_u8;
      case VALUE_TYPE_U16:
      case VALUE_TYPE_POKEMON_SPECIES:
         return *item->target.as_u16;
   }
   return 0;
}
static const void SetOptionValue(const struct CGOptionMenuItem* item, u16 value) {
   switch (item->value_type) {
      case VALUE_TYPE_BOOL8:
         *item->target.as_bool8 = value ? 1 : 0;
         break;
      case VALUE_TYPE_U8:
         *item->target.as_u8 = value;
         break;
      case VALUE_TYPE_U16:
      case VALUE_TYPE_POKEMON_SPECIES:
         *item->target.as_u16 = value;
         break;
   }
}
static const u8* GetOptionValueName(const struct CGOptionMenuItem* item, u16 value) {
   if (item->flags & (1 << MENUITEM_FLAG_IS_SUBMENU)) {
      return NULL;
   }
   if (item->flags & (1 << MENUITEM_FLAG_IS_ENUM)) {
      return item->values.named.name_strings[value];
   }
   
   if (item->value_type == VALUE_TYPE_BOOL8) {
      if (GetOptionValue(item) == 0) {
         return gText_lu_CGOptionValues_common_Disabled;
      }
      return gText_lu_CGOptionValues_common_Enabled;
   }
   
   if (item->flags & (1 << MENUITEM_FLAG_0_MEANS_DISABLED)) {
      return gText_lu_CGOptionValues_common_Disabled;
   }
   if (item->flags & (1 << MENUITEM_FLAG_0_MEANS_DEFAULT)) {
      return gText_lu_CGOptionValues_common_Default;
   }
   if (item->value_type == VALUE_TYPE_POKEMON_SPECIES) {
      u16 species = GetOptionValue(item);
      if (species == 0) {
         return gText_lu_CGOptionValues_common_None;
      }
      if (species >= NELEMS(gSpeciesNames)) {
         return gSpeciesNames[0];
      }
      return gSpeciesNames[species];
   }
   
   return NULL;
}
static u8 GetOptionValueCount(const struct CGOptionMenuItem* item) {
   if (item->flags & (1 << MENUITEM_FLAG_IS_SUBMENU)) {
      return 0;
   }
   if (item->flags & (1 << MENUITEM_FLAG_IS_ENUM)) {
      return item->values.named.count;
   }
   if (item->value_type == VALUE_TYPE_POKEMON_SPECIES) {
      return 0; // TODO
   }
   switch (item->value_type) {
      case VALUE_TYPE_BOOL8:
         return 2; // Disabled, Enabled
      case VALUE_TYPE_U8:
      case VALUE_TYPE_U16:
         return item->values.integral.max - item->values.integral.min;
   }
   return 0;
}
static u16 GetOptionMinValue(const struct CGOptionMenuItem* item) {
   if (item->value_type == VALUE_TYPE_POKEMON_SPECIES) {
      if (item->flags & (1 << MENUITEM_FLAG_POKEMON_SPECIES_ALLOW_0)) {
         return 0;
      }
      return 1;
   }
   switch (item->value_type) {
      case VALUE_TYPE_U8:
      case VALUE_TYPE_U16:
         return item->values.integral.min;
   }
   return 0;
}
static void CycleOptionSelectedValue(const struct CGOptionMenuItem* item, s8 by) {
   u16 selection;
   
   selection = GetOptionValue(item);
   if (item->value_type == VALUE_TYPE_BOOL8) {
      selection = !(selection & 1);
   } else if (item->value_type == VALUE_TYPE_POKEMON_SPECIES) {
      return;
   } else {
      u8  value_count;
      u16 minimum;
      
      value_count = GetOptionValueCount(item);
      minimum     = GetOptionMinValue(item);
      
      if (by < 0) {
         if (selection - minimum < -by) {
            selection = value_count - (-by - selection);
         } else {
            selection += by;
         }
      } else if (by > 0) {
         selection += by;
         selection %= value_count;
      }
      if (selection < minimum) {
         selection = minimum;
      }
   }
   
   SetOptionValue(item, selection);
}



//
// Menu state and associated funcs:
//

static struct MenuStackFrame {
   const u8* name;
   const struct CGOptionMenuItem* menu_items;
};

static struct MenuState {
   const struct MenuStackFrame breadcrumbs[8];
   u8 cursor_pos;
} sMenuState;

static void ResetMenuState(void);

static void UpdateDisplayedMenuName(void);
static void UpdateDisplayedMenuItems(void);

static const struct CGOptionMenuItem* GetCurrentMenuItemList(void);
static u8 GetScreenRowForCursorPos(void);

static void EnterSubmenu(const u8* submenu_name, const struct CGOptionMenuItem* submenu_items);
static bool8 TryExitSubmenu();

static void TryMoveMenuCursor(s8 by);


//
// Menu definitions:
//

static const struct CGOptionMenuItem sOverworldPoisonOptions[3] = {
   {  // Interval
      .name        = gText_lu_CGOptionName_OverworldPoison_Interval,
      .help_string = gText_lu_CGOptionHelp_OverworldPoison_Interval,
      .flags       = (1 << MENUITEM_FLAG_0_MEANS_DISABLED),
      .value_type = VALUE_TYPE_U8,
      .values = {
         .integral = {
            .min = 0,
            .max = 60,
         }
      },
      .target = {
         .as_u8 = &sTempOptions.overworld_poison_interval
      }
   },
   {  // Damage
      .name        = gText_lu_CGOptionName_OverworldPoison_Damage,
      .help_string = gText_lu_CGOptionHelp_OverworldPoison_Damage,
      .flags       = 0,
      .value_type = VALUE_TYPE_U16,
      .values = {
         .integral = {
            .min = 1,
            .max = 2000,
         }
      },
      .target = {
         .as_u16 = &sTempOptions.overworld_poison_damage
      }
   },
   sEndOfListSentinel,
};

static const struct CGOptionMenuItem sTopLevelMenu[3] = {
   {  // Start with running shoes
      .name        = gText_lu_CGOptionName_StartWithRunningShoes,
      .help_string = gText_lu_CGOptionHelp_StartWithRunningShoes,
      .flags       = 0,
      .value_type = VALUE_TYPE_BOOL8,
      .target = {
         .as_bool8 = &sTempOptions.start_with_running_shoes
      }
   },
   {  // SUBMENU: Overworld poison
      .name        = gText_lu_CGOptionCategory_OverworldPoison,
      .help_string = NULL,
      .flags       = (1 << MENUITEM_FLAG_IS_SUBMENU),
      .value_type = VALUE_TYPE_NONE,
      .target = {
         .submenu = &sOverworldPoisonOptions
      }
   },
   sEndOfListSentinel,
};

//
// Menu state and associated funcs:
//

static void ResetMenuState(void) {
   u8 i;
   
   sMenuState.breadcrumbs[0].name       = gText_lu_CGO_menuTitle;
   sMenuState.breadcrumbs[0].menu_items = &sTopLevelMenu;
   for(i = 1; i < 8; ++i) {
      sMenuState.breadcrumbs[i].name       = NULL;
      sMenuState.breadcrumbs[i].menu_items = NULL;
   }
   sMenuState.cursor_pos = 0;
}

static const struct CGOptionMenuItem* GetCurrentMenuItemList(void) {
   u8 i;
   const struct CGOptionMenuItem* items;
   
   items = &sTopLevelMenu;
   for(i = 0; i < 8; ++i) {
      if (sMenuState.breadcrumbs[i].menu_items != NULL) {
         items = sMenuState.breadcrumbs[i].menu_items;
      } else {
         break;
      }
   }
   return items;
}
static u8 GetScreenRowForCursorPos(void) {
   if (sMenuState.cursor_pos > MENU_ITEM_HALFWAY_ROW) {
      return MENU_ITEM_HALFWAY_ROW;
   }
   return sMenuState.cursor_pos;
}

static void EnterSubmenu(const u8* submenu_name, const struct CGOptionMenuItem* submenu_items) {
   u8    i;
   bool8 success;
   
   success = FALSE;
   for(i = 0; i < 8; ++i) {
      if (sMenuState.breadcrumbs[i] == NULL) {
         sMenuState.breadcrumbs[i].name       = name;
         sMenuState.breadcrumbs[i].menu_items = submenu_items;
         success = TRUE;
         break;
      }
   }
   if (!success) {
      // TODO: DebugPrint
      return;
   }
   
   sMenuState.cursor_pos = 0;
   
   UpdateDisplayedMenuName();
   UpdateDisplayedMenuItems();
}
static bool8 TryExitSubmenu() { // returns FALSE if at top-level menu
   u8    i;
   bool8 success;
   
   success = FALSE;
   for(int i = 7; i > 0; --i) {
      if (sMenuState.breadcrumbs[i].items != NULL) {
         sMenuState.breadcrumbs[i].name       = NULL;
         sMenuState.breadcrumbs[i].menu_items = NULL;
         success = TRUE;
         break;
      }
   }
   if (!success) {
      return FALSE; // in top-level menu
   }
   
   sMenuState.cursor_pos = 0; // TODO: use the scroll offset of the last entered submenu
   
   UpdateDisplayedMenuName();
   UpdateDisplayedMenuItems();
   
   return TRUE;
}

static void TryMoveMenuCursor(s8 by) {
   u8 items_count = GetMenuItemListCount(GetCurrentMenuItemList());
   
   if (by < 0) {
      if (sMenuState.cursor_pos < -by) {
         sMenuState.cursor_pos = items_count - 1;
      } else {
         sMenuState.cursor_pos += by;
      }
   }
   if (by > 0) {
      sMenuState.cursor_pos += by;
      if (sMenuState.cursor_pos >= items_count) {
         sMenuState.cursor_pos = 0;
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
static void HighlightCGOptionMenuItem(u8 selection);

static void DrawControls(void);
static void DrawHeaderText(void); // draw menu title
static void DrawBgWindowFrames(void);

static const u16 sOptionMenuText_Pal[] = INCBIN_U16("graphics/interface/option_menu_text.gbapal");
// note: this is only used in the Japanese release
static const u8 sEqualSignGfx[] = INCBIN_U8("graphics/interface/option_menu_equals_sign.4bpp");

static const struct WindowTemplate sOptionMenuWinTemplates[] = {
    [WIN_HEADER] = {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 1,
        .width = DISPLAY_TILE_WIDTH,
        .height = 2,
        .paletteNum = 11,
        .baseBlock = 0x074
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
    [WIN_KEYBINDS_STRIP] = {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = DISPLAY_TILE_WIDTH,
        .height = 2,
        .paletteNum = 11,
        .baseBlock = 0x074
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
           CopyWindowToVram(WIN_OPTIONS, COPYWIN_FULL);
           gMain.state++;
       case 9:
           DrawControls();
           DrawBgWindowFrames();
           gMain.state++;
           break;
       case 10:
       {
           u8 taskId = CreateTask(Task_CGOptionMenuFadeIn, 0);

           ResetMenuState();
           sTempOptions = gCustomGameOptions;
           UpdateDisplayedMenuItems();
           HighlightCGOptionMenuItem(sMenuState.cursor_pos);

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

static void Task_CGOptionMenuFadeIn(u8 taskId) {
   if (!gPaletteFade.active)
      gTasks[taskId].func = Task_CGOptionMenuProcessInput;
}

static void Task_CGOptionMenuProcessInput(u8 taskId) {
   if (JOY_NEW(A_BUTTON)) {
      const struct CGOptionsMenuItem* items;
      const struct CGOptionsMenuItem* item;
      
      items = GetCurrentMenuItemList();
      item  = items[sMenuState.cursor_pos];
      
      if (item.flags & (1 << MENUITEM_FLAG_IS_SUBMENU)) {
         EnterSubmenu(item->name, item->target.submenu);
         return;
      }
      
      //
      // TODO: For Pokemon species, show a special screen to select them
      //
      return;
   }
   if (JOY_NEW(B_BUTTON)) {
      if (TryExitSubmenu()) {
         return;
      }
      //
      // We're in the top-level menu. Back out.
      //
      gTasks[taskId].func = Task_CGOptionMenuSave;
      return;
   }
   if (JOY_NEW(R_BUTTON)) {
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
      HighlightCGOptionMenuItem(sMenuState.cursor_pos);
      return;
   }
   if (JOY_NEW(DPAD_DOWN)) {
      TryMoveMenuCursor(1);
      UpdateDisplayedMenuItems();
      HighlightCGOptionMenuItem(sMenuState.cursor_pos);
      return;
   }
   
   // Left/Right: Cycle option value
   {
      s8 by = JOY_NEW(DPAD_RIGHT) ? 1 : 0;
      if (!by) {
         by = JOY_NEW(DPAD_LEFT) ? -1 : 0;
      }
      if (by) {
         const struct CGOptionsMenuItem* items = GetCurrentMenuItemList();
         
         CycleOptionSelectedValue(&items[sMenuState.cursor_pos], by);
         DrawMenuItem(&items[sMenuState.cursor_pos], GetScreenRowForCursorPos());
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
   index = GetScreenRowForCursorPos(index);
   
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
   for(i = 0; i < 8; ++i) {
      if (sMenuState.breadcrumbs[i].name) {
         title = sMenuState.breadcrumbs[i].name;
      } else {
         break;
      }
   }
   
   FillWindowPixelBuffer(WIN_HEADER, PIXEL_FILL(1));
   AddTextPrinterParameterized(WIN_HEADER, FONT_NORMAL, title, 8, 1, TEXT_SKIP_DRAW, NULL);
   CopyWindowToVram(WIN_HEADER, COPYWIN_FULL);
}

static void DrawMenuItem(const struct CGOptionMenuItem* item, u8 row) {
   u16 text_width;
   u16 x_offset;
   const u8* value_text;
   
   // Name
   AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, item->name, 8, (row * 16) + 1, TEXT_SKIP_DRAW, NULL);
   
   // Value
   value_text = GetOptionValueName(item, GetOptionValue(item));
   if (value_text != NULL) {
      text_width = GetStringWidth(FONT_NORMAL, value_text, 0);
      //
      // left  edge is at 104px
      // right edge is at 198px
      // let's center the option value within the available space, for now
      // (it'll look neater once we display left/right arrows next to it)
      x_offset   = (94 - text_width) / 2 + 104;
      //
      AddTextPrinterParameterized(
         WIN_OPTIONS,
         FONT_NORMAL,
         value_text,
         x_offset,
         (row * 16) + 1,
         TEXT_SKIP_DRAW,
         NULL
      );
   } else if (item->flags & (1 << MENUITEM_FLAG_IS_SUBMENU)) {
      // TODO: For submenus, draw an icon like ">>" in place of the value
   }
}

static void UpdateDisplayedMenuItems(void) {
   u8 i;
   u8 count;
   u8 scroll;
   const struct CGOptionMenuItem* items;
   
   items  = GetCurrentMenuItemList();
   count  = GetMenuItemListCount(items);
   scroll = sMenuState.cursor_pos;
   if (count >= MAX_MENU_ITEMS_VISIBLE_AT_ONCE) {
      if (scroll + MAX_MENU_ITEMS_VISIBLE_AT_ONCE > count) {
         scroll = count - MAX_MENU_ITEMS_VISIBLE_AT_ONCE;
      }
   }
   
   FillWindowPixelBuffer(WIN_OPTIONS, PIXEL_FILL(1));
   
   for(i = 0; i < MAX_MENU_ITEMS_VISIBLE_AT_ONCE; ++i) {
      if (i + scroll >= count) {
         break;
      }
      DrawMenuItem(&items[i + scroll]);
   }
   
   // 100% necessary or we see a blank window when we scroll
   CopyWindowToVram(WIN_OPTIONS, COPYWIN_GFX);
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
    /*//
    // Draw title window frame
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_L,  1,  0,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_EDGE,      2,  0, 27,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_R, 28,  0,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_LEFT_EDGE,     1,  1,  1,  2,  7);
    FillBgTilemapBufferRect(1, TILE_RIGHT_EDGE,   28,  1,  1,  2,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_L,  1,  3,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_EDGE,      2,  3, 27,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_R, 28,  3,  1,  1,  7);
    //*/

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

static void DrawControls(void) {
   const u8 color[3] = { TEXT_DYNAMIC_COLOR_6, TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY };

   FillWindowPixelBuffer(sOptionMenuWinTemplates->windows[WIN_KEYBINDS_STRIP], PIXEL_FILL(15));
   AddTextPrinterParameterized3(sOptionMenuWinTemplates->windows[WIN_KEYBINDS_STRIP], FONT_SMALL, 2, 1, color, 0, gText_lu_CGO_keybinds);
   PutWindowTilemap(sOptionMenuWinTemplates->windows[WIN_KEYBINDS_STRIP]);
   CopyWindowToVram(sOptionMenuWinTemplates->windows[WIN_KEYBINDS_STRIP], COPYWIN_FULL);
}