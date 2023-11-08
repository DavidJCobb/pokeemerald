
# Convenient constants

`DISPLAY_WIDTH` is 240
`DISPLAY_HEIGHT` is 160

`TILE_WIDTH` and `TILE_HEIGHT` are 8

`DISPLAY_TILE_WIDTH` and `DISPLAY_TILE_HEIGHT` are the screen size in tiles


# The overall organization of a menu

Menus are typically organized into multiple "windows," which define regions that various game functions can paint to. Windows operate using background tiles (VRAM is divided into "background" and "object" assets and tiles).

## Backgrounds

A menu will need to define its background layers. A typical background template looks like this:

```c
static const struct BgTemplate sMyTemplate = {
   .bg = 0,
   //
   .charBaseIndex = 1,
   .mapBaseIndex  = 31,
   .screenSize    = 0,
   .paletteMode   = 0,
   .priority      = 1,
   .baseTile      = 0
};
```

The fields are as follows:

<dl>
   <dt><code>bg</code></dt>
      <dd>Identifies which of the four background layers to use.</dd>
   <dt><code>charBaseIndex</code></dt>
      <dd>"Character Base Block," measured in 16KiB units.</dd>
   <dt><code>mapBaseIndex</code></dt>
      <dd>"Screen Base Block," measured in 2KiB units.</dd>
   <dt><code>screenSize</code></dt>
      <dd>An enumeration which controls the screen size. When the backgrounds are in "text mode," the usual for menus, values in the range [0, 3] represent 256x256px, 512x256px, 256x512px, and 512x512px. When the backgrounds are in "rotation/scaling mode," values in the range [0, 3] represent 128x128px, 256x256px, 512x512px, and 1024x1024px.</dd>
   <dt><code>paletteMode</code></dt>
      <dd>An enumeration. Value 0 indicates that this background should treat the background palette data as 16 palettes each containing 16 colors. Value 1 indicates that this background should treat the background palette data as a single 256-color palette.</dd>
   <dt><code>priority</code></dt>
      <dd>Controls the order in which backgrounds are rendered. Background with lower numbers draw overtop backgrounds with higher numbers.</dd>
   <dt><code>baseTile</code></dt>
      <dd>As with paint windows (see below), this specifies where in VRAM this background layer's tiles will be stored. Backgrounds can clobber each other when loading tiles, if you aren't careful with this.</dd>
</dl>

Game Freak more commonly defines backgrounds in a set, like this:

```c
static const struct BgTemplate sOptionMenuBgTemplates[] =
{
    {
        .bg = 1,
        .charBaseIndex = 1,
        .mapBaseIndex  = 30,
        .screenSize    = 0,
        .paletteMode   = 0,
        .priority      = 0,
        .baseTile      = 0
    },
    {
        .bg = 0,
        .charBaseIndex = 1,
        .mapBaseIndex  = 31,
        .screenSize    = 0,
        .paletteMode   = 0,
        .priority      = 1,
        .baseTile      = 0
    }
};
```

They then set these up with a call like this:

```c
InitBgsFromTemplates(0, sOptionMenuBgTemplates, ARRAY_COUNT(sOptionMenuBgTemplates));
ChangeBgX(0, 0, BG_COORD_SET);
ChangeBgY(0, 0, BG_COORD_SET);
ChangeBgX(1, 0, BG_COORD_SET);
ChangeBgY(1, 0, BG_COORD_SET);
ChangeBgX(2, 0, BG_COORD_SET);
ChangeBgY(2, 0, BG_COORD_SET);
ChangeBgX(3, 0, BG_COORD_SET);
ChangeBgY(3, 0, BG_COORD_SET);
```

## Windows

Windows define regions that various game functions can paint to. They operate using background tiles (VRAM is divided into "background" and "object" assets and tiles). When you heaer the term "window," you probably think of one of the bordered dialog boxes or text windows shown in-game. At the engine level, however, a "window" only defines what those familiar with CSS would think of as the "content-box" for a dialog box or dialogue window. The "border-box" is typically drawn manually by other functions. A library for in-game dialogue windows exists (`text_window.h`), but GUIs like the Options menu will typically load and draw border tiles manually, placing them outside of the content-box.

A typical window template looks like this:

```c
static const struct WindowTemplate sMyTemplate = {
   .bg          = 1,
   .tilemapLeft = 3,
   .tilemapTop  = 10,
   .width       = 19,
   .height      = 8,
   .paletteNum  = 10,
   .baseBlock   = 0x030
};
```

The fields are as follows:

<dl>
   <dt><code>bg</code></dt>
      <dd>Identifies which of the four background layers this window draws to. This is generally important when using blending, dimming/brightening, and other visual effects.</dd>
   <dt><code>tilemapLeft</code></dt>
      <dd>
         Positions the left edge of the window's content-box. This is measured in 4x4px tiles.
      </dd>
   <dt><code>tilemapTop</code></dt>
      <dd>
         Positions the top edge of the window's content-box. This is measured in 4x4px tiles, so a position of 20 (160px / 4) would place the window fully off the top edge of the screen.
      </dd>
   <dt><code>width</code></dt>
      <dd>
         Defines the width of the window's content-box. This is measured in 4x4px tiles.
      </dd>
   <dt><code>height</code></dt>
      <dd>
         Defines the width of the window's content-box. This is measured in 4x4px tiles.
      </dd>
   <dt><code>paletteNum</code></dt>
      <dd>
         Specifies which of the available 16 background palettes the window will use. You need to load palettes yourself, and need to manually assign them to background palette ID numbers.
      </dd>
   <dt><code>baseBlock</code></dt>
      <dd>
         <p>Specifies where in VRAM this menu's tiles will be loaded; this index is relative to the background definition's <code>baseTile</code>. This is important: you must make sure that windows' tiles don't share overlapping regions in memory, or else drawing to one window will clobber some or all of the other window's content-box. The number of tiles that a window uses will be <code>width * height</code>, so one easy way to prevent clobbering is to define Window 1's <code>baseBlock</code> as <code>WINDOW_0_WIDTH * WINDOW_0_HEIGHT</code>, and then define Window 2's <code>baseBlock</code> as <code>WINDOW_1_BASE_BLOCK + (WINDOW_1_WIDTH * WINDOW_1_HEIGHT)</code>, and so on.</p>
         <p>Importantly: none of these should be 0. By default, a background is filled with tile zero, which is typically filled with palette 0 color 0; the zeroth color in each palette is treated as transparent except when there's nothing to show behind it. If you start a window at tile zero within its background, then your window will paint over that &mdash; and in doing so, overwrite everything in the background layer that <em.doesn't</em> get painted over. Or something like that. I'm still, uh, putting this all together mentally.</p>
      </dd>
</dl>

Game Freak more commonly defines windows in a set; a typical example (from `naming_screen.c`):

```c
enum {
    WIN_KB_PAGE_1, // Which of these two windows is in front is cycled as the player swaps
    WIN_KB_PAGE_2, // Initially WIN_KB_PAGE_1 is in front, with WIN_KB_PAGE_2 on deck
    WIN_TEXT_ENTRY,
    WIN_TEXT_ENTRY_BOX,
    WIN_BANNER, // controls displayed along the top
    WIN_COUNT,
};

static const struct WindowTemplate sWindowTemplates[WIN_COUNT + 1] =
{
    [WIN_KB_PAGE_1] = {
        .bg = 1,
        .tilemapLeft = 3,
        .tilemapTop = 10,
        .width = 19,
        .height = 8,
        .paletteNum = 10,
        .baseBlock = 0x030
    },
    [WIN_KB_PAGE_2] = {
        .bg = 2,
        .tilemapLeft = 3,
        .tilemapTop = 10,
        .width = 19,
        .height = 8,
        .paletteNum = 10,
        .baseBlock = 0x0C8
    },
    [WIN_TEXT_ENTRY] = {
        .bg = 3,
        .tilemapLeft = 8,
        .tilemapTop = 6,
        .width = 17,
        .height = 2,
        .paletteNum = 10,
        .baseBlock = 0x030
    },
    [WIN_TEXT_ENTRY_BOX] = {
        .bg = 3,
        .tilemapLeft = 8,
        .tilemapTop = 4,
        .width = 17,
        .height = 2,
        .paletteNum = 10,
        .baseBlock = 0x052
    },
    [WIN_BANNER] = {
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
```

They can then pass `sWindowTemplates` as an argument to `InitWindows`. The purpose of `DUMMY_WIN_TEMPLATE` at the end is to act as a list terminator, since `InitWindows` only takes the start of the list as an argument.

Occasionally, you may need to modify a window's attributes after its creation. You can do that using the `SetWindowAttribute` function defined in `window.h`. The main menu is an example of a place where this *could* be used, because it uses a window for each menu item and the menu item arrangement can vary (i.e. "Continue" and "Mystery Gift" are conditionally shown). The vanilla main menu just defines redundant windows (e.g. one window for every position the "Options" menu item can be in), but my rewrite of it defines a dedicated window for each menu item, and I reposition them with `SetWindowAttribute(i, WINDOW_TILEMAP_TOP, item_top + MENU_TOP);` based on which menu items are currently visible.


# Common tasks

## Drawing anything

If your goal is to repaint an entire window, then start by calling `FillWindowPixelBuffer(WINDOW_ID_HERE, PIXEL_FILL(PALETTE_INDEX_HERE));`, passing the window ID you wish to use and the index of a color in its palette to use. That'll clear the entire window and paint in the color you've specified. Then, perform your drawing tasks. This may involve painting tiles directly to the window or using a text printer (see the section on displaying text). Once you've made your changes, call `CopyWindowToVram(WINDOW_ID_HERE, COPYWIN_GFX);` to update the display of your window.

If your goal is to repaint just part of a window, you can use these functions instead:

```c
// Coordinates are relative to the window's content-box.
FillWindowPixelRect(window_id, PIXEL_FILL(palette_color), x_px, y_px, width_px, height_px);
CopyWindowRectToVram(window_id, COPYWIN_FULL, x_px, y_px, width_px, height_px);
```

## Drawing the user's dialog window frame

First, you'll need to load the window frame:

```c
#define BACKGROUND_ID                1 // background ID you plan on drawing the frame onto
#define BACKGROUND_PALETTE_BOX_FRAME 7 // background palette ID to use; you have room for 16 palettes

LoadBgTiles(BACKGROUND_ID, GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->tiles, 0x120, 0x1A2);
LoadPalette(GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->pal, BG_PLTT_ID(BACKGROUND_PALETTE_BOX_FRAME), PLTT_SIZE_4BPP);
```

The arguments to `LoadBgTiles` are:

* The background ID.
* The tile data pointer.
* The tile data size (in bytes?).
* A tile ID in this background to load it to; this is added to the background's `baseTile`.

Once the tiles are loaded, they'll have the following arrangement:

1. Corner, top left
2. Edge, top
3. Corner, top right,
4. Edge, left
5. Edge, right
6. Corner, bottom left
7. Edge, bottom
8. Corner, bottom right

If we load to tile ID 0x1A2 within our chosen background, as the Options menu does, then we end up with:

```c
#define TILE_TOP_CORNER_L 0x1A2
#define TILE_TOP_EDGE     0x1A3
#define TILE_TOP_CORNER_R 0x1A4
#define TILE_LEFT_EDGE    0x1A5
#define TILE_RIGHT_EDGE   0x1A7
#define TILE_BOT_CORNER_L 0x1A8
#define TILE_BOT_EDGE     0x1A9
#define TILE_BOT_CORNER_R 0x1AA
```

We can then draw a window frame around something using this code:

```c
// Args: background ID; tile ID; x; y; width; height; palette ID

FillBgTilemapBufferRect(BACKGROUND_ID, TILE_TOP_CORNER_L,  1,  0,  1,  1,  BACKGROUND_PALETTE_BOX_FRAME);
FillBgTilemapBufferRect(BACKGROUND_ID, TILE_TOP_EDGE,      2,  0, 27,  1,  BACKGROUND_PALETTE_BOX_FRAME);
FillBgTilemapBufferRect(BACKGROUND_ID, TILE_TOP_CORNER_R, 28,  0,  1,  1,  BACKGROUND_PALETTE_BOX_FRAME);
FillBgTilemapBufferRect(BACKGROUND_ID, TILE_LEFT_EDGE,     1,  1,  1,  2,  BACKGROUND_PALETTE_BOX_FRAME);
FillBgTilemapBufferRect(BACKGROUND_ID, TILE_RIGHT_EDGE,   28,  1,  1,  2,  BACKGROUND_PALETTE_BOX_FRAME);
FillBgTilemapBufferRect(BACKGROUND_ID, TILE_BOT_CORNER_L,  1,  3,  1,  1,  BACKGROUND_PALETTE_BOX_FRAME);
FillBgTilemapBufferRect(BACKGROUND_ID, TILE_BOT_EDGE,      2,  3, 27,  1,  BACKGROUND_PALETTE_BOX_FRAME);
FillBgTilemapBufferRect(BACKGROUND_ID, TILE_BOT_CORNER_R, 28,  3,  1,  1,  BACKGROUND_PALETTE_BOX_FRAME);
```

More generically:

```c
// TODO: some of these defines need fixing

#define FRAME_X 1
#define FRAME_Y 0
#define FRAME_OUTER_WIDTH  29
#define FRAME_OUTER_HEIGHT  4

#define FRAME_X_END (FRAME_X + FRAME_OUTER_WIDTH  - 1)
#define FRAME_Y_END (FRAME_Y + FRAME_OUTER_HEIGHT - 1)

#define FRAME_X_MID (FRAME_OUTER_WIDTH  - 2)
#define FRAME_Y_MID (FRAME_OUTER_HEIGHT - 2)

FillBgTilemapBufferRect(BACKGROUND_ID, TILE_TOP_CORNER_L, FRAME_X,      FRAME_Y,      1,            1,  BACKGROUND_PALETTE_BOX_FRAME);
FillBgTilemapBufferRect(BACKGROUND_ID, TILE_TOP_EDGE,     FRAME_X + 1,  FRAME_Y,      FRAME_X_MID,  1,  BACKGROUND_PALETTE_BOX_FRAME);
FillBgTilemapBufferRect(BACKGROUND_ID, TILE_TOP_CORNER_R, FRAME_X_END,  FRAME_Y,      1,            1,  BACKGROUND_PALETTE_BOX_FRAME);
FillBgTilemapBufferRect(BACKGROUND_ID, TILE_LEFT_EDGE,    FRAME_X,      FRAME_Y + 1,  1,  FRAME_Y_MID,  BACKGROUND_PALETTE_BOX_FRAME);
FillBgTilemapBufferRect(BACKGROUND_ID, TILE_RIGHT_EDGE,   FRAME_X_END,  FRAME_Y + 1,  1,  FRAME_Y_MID,  BACKGROUND_PALETTE_BOX_FRAME);
FillBgTilemapBufferRect(BACKGROUND_ID, TILE_BOT_CORNER_L, FRAME_X,      FRAME_X_END,  1,            1,  BACKGROUND_PALETTE_BOX_FRAME);
FillBgTilemapBufferRect(BACKGROUND_ID, TILE_BOT_EDGE,     FRAME_X + 1,  FRAME_X_END,  FRAME_X_MID,  1,  BACKGROUND_PALETTE_BOX_FRAME);
FillBgTilemapBufferRect(BACKGROUND_ID, TILE_BOT_CORNER_R, FRAME_X_END,  FRAME_X_END,  1,            1,  BACKGROUND_PALETTE_BOX_FRAME);
```

## Printing text to the screen

Text display relies on "text printers," which are state machines used to manage text-printing animations as well as any formatting codes within a string and their effects on the text that comes after them. Every text printer is scoped to an on-screen "window" object, which you reference by numeric ID.

If you print text to the screen instantly, without animations or delays, then there's no limit on how much text you can print into any given window: the game creates a stack-allocated temporary text printer, runs it immediately, and then discards it. If you use a text printing speed other than 0 (which prints instantly and redraws the window) or TEXT_SKIP_DRAW (which prints instantly but doesn't update VRAM), however, then a peristent text printer is created, and you can only have so many of those at a time (at most 32, which is also the max window count). Persistent text printers can be given a callback &mdash; a function pointer to run when text printing is complete.

A typical text-printing call might look like this:

```c
// background, text-shadow, text color
const u8 sTextColors[3] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_LIGHT_RED, TEXT_COLOR_RED};

// window-relative coordinates, measured in pixels.
u8 x = (POKEMON_NAME_LENGTH * 4) + 64;
u8 y = 1;

/*
   You can and should use a string defined separately (follow the example set by `strings.h` 
   and `strings.c`), but if you need to generate text programmatically, it is useful to know 
   how to do that. Remember that Game Freak *does* have string helper functions defined in 
   `string_util.h`, like `StringCopy`, which you can use to stitch multiple strings together.
*/
u8 text[6];
text[0] = CHAR_H;
text[1] = CHAR_e;
text[2] = CHAR_l;
text[3] = CHAR_l;
text[4] = CHAR_o;
text[5] = EOS; // end-of-string terminator, which is 0xFF, not 0x00

AddTextPrinterParameterized3(WINDOW_ID, FONT_NORMAL, x, y, sTextColors, TEXT_SKIP_DRAW, text);
```

There are multiple different function signatures for text printing, with the most basic defined in `text.h` and ones like `AddTextPrinterParameterized3` defined in `menu.h`. C doesn't have function overloads, hence the numeric suffix on that one. These functions typically return the numeric ID of whatever text printer slot they use.






# Naming menu

## Setup tasks (via the initial main callback-2)

### State 0

```c
ResetVHBlank();
NamingScreen_Init();
```

### State 1

```c
NamingScreen_InitBGs();
```

This function does the following:

* Clear VRAM, OAM, and palettes.
* Reset backgrounds within the GPU, and then initialize them from menu-specific definitions.
* Move all backgrounds to coordinates (0, 0).
* Set up textbox windows and printers.
* Add all windows -- not as a batch, but in a loop, interestingly enough.
* Set up background blending (to do what?).
* Clear all tilemap buffers.

```c
DmaClearLarge16(3, (void *)VRAM, VRAM_SIZE, 0x1000);
DmaClear32(3, (void *)OAM, OAM_SIZE);
DmaClear16(3, (void *)PLTT, PLTT_SIZE);

SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0);
ResetBgsAndClearDma3BusyFlags(0);
InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));

ChangeBgX(0, 0, BG_COORD_SET);
ChangeBgY(0, 0, BG_COORD_SET);
ChangeBgX(1, 0, BG_COORD_SET);
ChangeBgY(1, 0, BG_COORD_SET);
ChangeBgX(2, 0, BG_COORD_SET);
ChangeBgY(2, 0, BG_COORD_SET);
ChangeBgX(3, 0, BG_COORD_SET);
ChangeBgY(3, 0, BG_COORD_SET);

InitStandardTextBoxWindows();
InitTextBoxGfxAndPrinters();

for (i = 0; i < WIN_COUNT; i++)
  sNamingScreen->windows[i] = AddWindow(&sWindowTemplates[i]);

SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_EFFECT_BLEND | BLDCNT_TGT2_BG1 | BLDCNT_TGT2_BG2);
SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(12, 8));

SetBgTilemapBuffer(1, sNamingScreen->tilemapBuffer1);
SetBgTilemapBuffer(2, sNamingScreen->tilemapBuffer2);
SetBgTilemapBuffer(3, sNamingScreen->tilemapBuffer3);

FillBgTilemapBufferRect_Palette0(1, 0, 0, 0, 0x20, 0x20);
FillBgTilemapBufferRect_Palette0(2, 0, 0, 0, 0x20, 0x20);
FillBgTilemapBufferRect_Palette0(3, 0, 0, 0, 0x20, 0x20);
```

### State 2

Resets whatever visual effects Game Freak expected to be in effect at this point.

```c
ResetPaletteFade();
```

### State 3

Resets sprite state and palettes.

```c
ResetSpriteData();
FreeAllSpritePalettes();
```

### State 4

```c
ResetTasks();
```

### State 5

```c
LoadPalettes();
```

The source for that function is here:

```c
static void LoadPalettes(void)
{
    LoadPalette(gNamingScreenMenu_Pal, BG_PLTT_ID(0), sizeof(gNamingScreenMenu_Pal));
    LoadPalette(sKeyboard_Pal, BG_PLTT_ID(10), sizeof(sKeyboard_Pal));
    LoadPalette(GetTextWindowPalette(2), BG_PLTT_ID(11), PLTT_SIZE_4BPP);
}
```

The menu's overall palette loads to slot 0, and the keyboard palette loads to slot 10. Slot 11 is used for the keybind strip along the top edge of the screen, which tells you what (most of) the menu's button mappings are.

### State 6

```c
LoadGfx();
```

### State 7

```c
CreateSprites();
UpdatePaletteFade();
NamingScreen_ShowBgs();
```

`CreateSprites` calls a handful of functions to create the different animated sprites on the menu, such as the cursor, the page swap buttons, and the "back" and "OK" buttons.

### State 8

```c
CreateHelperTasks();
CreateNamingScreenTask();
```

The latter function does the following. The task it creates handles menu fades and interaction.

```c
CreateTask(Task_NamingScreen, 2);
SetMainCallback2(CB2_NamingScreen);
```



## Sprite creation and control

Seems like sprites have arbitrary data fields associated with them similarly to tasks, for use by whatever callbacks they set.

### Menu cursor

```c
// Sprite data for the the cursor
#define sX          data[0]
#define sY          data[1]
#define sPrevX      data[2]
#define sPrevY      data[3]
#define sInvisible  data[4] & 0x00FF
#define sFlashing   data[4] & 0xFF00
#define sColor      data[5]
#define sColorIncr  data[6]
#define sColorDelay data[7]

static void CreateCursorSprite(void)
{
    sNamingScreen->cursorSpriteId = CreateSprite(&sSpriteTemplate_Cursor, 38, 88, 1);
    SetCursorInvisibility(TRUE);
    gSprites[sNamingScreen->cursorSpriteId].oam.priority = 1;
    gSprites[sNamingScreen->cursorSpriteId].oam.objMode = ST_OAM_OBJ_BLEND;
    gSprites[sNamingScreen->cursorSpriteId].sColorIncr = 1; // ? immediately overwritten
    gSprites[sNamingScreen->cursorSpriteId].sColorIncr = 2;
    SetCursorPos(0, 0);
}

static void SetCursorPos(s16 x, s16 y)
{
    struct Sprite *cursorSprite = &gSprites[sNamingScreen->cursorSpriteId];

    if (x < sPageColumnCounts[CurrentPageToKeyboardId()])
        cursorSprite->x = sPageColumnXPos[CurrentPageToKeyboardId()][x] + 38;
    else
        cursorSprite->x = 0;

    cursorSprite->y = y * 16 + 88;
    cursorSprite->sPrevX = cursorSprite->sX;
    cursorSprite->sPrevY = cursorSprite->sY;
    cursorSprite->sX = x;
    cursorSprite->sY = y;
}

static void SetCursorInvisibility(bool8 invisible)
{
    gSprites[sNamingScreen->cursorSpriteId].data[4] &= 0xFF00;
    gSprites[sNamingScreen->cursorSpriteId].data[4] |= invisible; // sInvisible
    StartSpriteAnim(&gSprites[sNamingScreen->cursorSpriteId], 0);
}

static void SetCursorFlashing(bool8 flashing)
{
    gSprites[sNamingScreen->cursorSpriteId].data[4] &= 0xFF;
    gSprites[sNamingScreen->cursorSpriteId].data[4] |= flashing << 8; // sFlashing
}

static void SquishCursor(void)
{
    StartSpriteAnim(&gSprites[sNamingScreen->cursorSpriteId], 1);
}

static bool8 IsCursorAnimFinished(void)
{
    return gSprites[sNamingScreen->cursorSpriteId].animEnded;
}
```

### Page-swap sprites

```c
static bool8 PageSwapSprite_Init(struct Sprite *);
static bool8 PageSwapSprite_Idle(struct Sprite *);
static bool8 PageSwapSprite_SlideOff(struct Sprite *);
static bool8 PageSwapSprite_SlideOn(struct Sprite *);

#define sState          data[0]
#define sPage           data[1]
#define sTextSpriteId   data[6]
#define sButtonSpriteId data[7]

static void CreatePageSwapButtonSprites(void)
{
    u8 frameSpriteId;
    u8 textSpriteId;
    u8 buttonSpriteId;

    frameSpriteId = CreateSprite(&sSpriteTemplate_PageSwapFrame, 204, 88, 0);
    sNamingScreen->swapBtnFrameSpriteId = frameSpriteId;
    SetSubspriteTables(&gSprites[frameSpriteId], sSubspriteTable_PageSwapFrame);
    gSprites[frameSpriteId].invisible = TRUE;

    textSpriteId = CreateSprite(&sSpriteTemplate_PageSwapText, 204, 84, 1);
    gSprites[frameSpriteId].sTextSpriteId = textSpriteId;
    SetSubspriteTables(&gSprites[textSpriteId], sSubspriteTable_PageSwapText);
    gSprites[textSpriteId].invisible = TRUE;

    buttonSpriteId = CreateSprite(&sSpriteTemplate_PageSwapButton, 204, 83, 2);
    gSprites[buttonSpriteId].oam.priority = 1;
    gSprites[frameSpriteId].sButtonSpriteId = buttonSpriteId;
    gSprites[buttonSpriteId].invisible = TRUE;
}

static void StartPageSwapButtonAnim(void)
{
    struct Sprite *sprite = &gSprites[sNamingScreen->swapBtnFrameSpriteId];

    sprite->sState = 2; // go to PageSwapSprite_SlideOff
    sprite->sPage = sNamingScreen->currentPage;
}

static u8 (*const sPageSwapSpriteFuncs[])(struct Sprite *) =
{
    PageSwapSprite_Init,
    PageSwapSprite_Idle,
    PageSwapSprite_SlideOff,
    PageSwapSprite_SlideOn,
};

static void SpriteCB_PageSwap(struct Sprite *sprite)
{
    while (sPageSwapSpriteFuncs[sprite->sState](sprite));
}

static bool8 PageSwapSprite_Init(struct Sprite *sprite)
{
    struct Sprite *text = &gSprites[sprite->sTextSpriteId];
    struct Sprite *button = &gSprites[sprite->sButtonSpriteId];

    SetPageSwapButtonGfx(PageToNextGfxId(sNamingScreen->currentPage), text, button);
    sprite->sState++;
    return FALSE;
}

static bool8 PageSwapSprite_Idle(struct Sprite *sprite)
{
    return FALSE;
}

static bool8 PageSwapSprite_SlideOff(struct Sprite *sprite)
{
    struct Sprite *text = &gSprites[sprite->sTextSpriteId];
    struct Sprite *button = &gSprites[sprite->sButtonSpriteId];

    text->y2++;
    if (text->y2 > 7)
    {
        sprite->sState++;
        text->y2 = -4;
        text->invisible = TRUE;
        SetPageSwapButtonGfx(PageToNextGfxId(((u8)sprite->sPage + 1) % KBPAGE_COUNT), text, button);
    }
    return FALSE;
}

static bool8 PageSwapSprite_SlideOn(struct Sprite *sprite)
{
    struct Sprite *text = &gSprites[sprite->sTextSpriteId];

    text->invisible = FALSE;
    text->y2++;
    if (text->y2 >= 0)
    {
        text->y2 = 0;
        sprite->sState = 1; // go to PageSwapSprite_Idle
    }
    return FALSE;
}

static const u16 sPageSwapPalTags[] = {
    [PAGE_SWAP_UPPER]  = PALTAG_PAGE_SWAP_UPPER,
    [PAGE_SWAP_OTHERS] = PALTAG_PAGE_SWAP_OTHERS,
    [PAGE_SWAP_LOWER]  = PALTAG_PAGE_SWAP_LOWER
};

static const u16 sPageSwapGfxTags[] = {
    [PAGE_SWAP_UPPER]  = GFXTAG_PAGE_SWAP_UPPER,
    [PAGE_SWAP_OTHERS] = GFXTAG_PAGE_SWAP_OTHERS,
    [PAGE_SWAP_LOWER]  = GFXTAG_PAGE_SWAP_LOWER
};

static void SetPageSwapButtonGfx(u8 page, struct Sprite *text, struct Sprite *button)
{
    button->oam.paletteNum = IndexOfSpritePaletteTag(sPageSwapPalTags[page]);
    text->sheetTileStart = GetSpriteTileStartByTag(sPageSwapGfxTags[page]);
    text->subspriteTableNum = page;
}

#undef sState
#undef sPage
#undef sTextSpriteId
#undef sButtonSpriteId
```

### Showing an icon for the things you're naming or giving input to

The PC icon uses the same palette as the menu itself. If I had to guess, `CreateObjectGraphicsSprite` is probably responsible for loading the appropriate palettes for Brendan, May, Walda, and her father, while `CreateMonIcon` probably grabs the right sprite and palette for the Pokemon to be named. These would be object palettes, not background palettes, and so can largely be ignored when trying to figure out how to set up the menu's own palettes.

```c
static void NamingScreen_NoIcon(void);
static void NamingScreen_CreatePlayerIcon(void);
static void NamingScreen_CreatePCIcon(void);
static void NamingScreen_CreateMonIcon(void);
static void NamingScreen_CreateWaldaDadIcon(void);

static void (*const sIconFunctions[])(void) =
{
    NamingScreen_NoIcon,
    NamingScreen_CreatePlayerIcon,
    NamingScreen_CreatePCIcon,
    NamingScreen_CreateMonIcon,
    NamingScreen_CreateWaldaDadIcon,
};

static void CreateInputTargetIcon(void)
{
    sIconFunctions[sNamingScreen->template->iconFunction]();
}

static void NamingScreen_NoIcon(void)
{

}

static void NamingScreen_CreatePlayerIcon(void)
{
    u8 rivalGfxId;
    u8 spriteId;

    rivalGfxId = GetRivalAvatarGraphicsIdByStateIdAndGender(PLAYER_AVATAR_STATE_NORMAL, sNamingScreen->monSpecies);
    spriteId = CreateObjectGraphicsSprite(rivalGfxId, SpriteCallbackDummy, 56, 37, 0);
    gSprites[spriteId].oam.priority = 3;
    StartSpriteAnim(&gSprites[spriteId], ANIM_STD_GO_SOUTH);
}

static void NamingScreen_CreatePCIcon(void)
{
    u8 spriteId;

    spriteId = CreateSprite(&sSpriteTemplate_PCIcon, 56, 41, 0);
    SetSubspriteTables(&gSprites[spriteId], sSubspriteTable_PCIcon);
    gSprites[spriteId].oam.priority = 3;
}

static void NamingScreen_CreateMonIcon(void)
{
    u8 spriteId;

    LoadMonIconPalettes();
    spriteId = CreateMonIcon(sNamingScreen->monSpecies, SpriteCallbackDummy, 56, 40, 0, sNamingScreen->monPersonality, 1);
    gSprites[spriteId].oam.priority = 3;
}

static void NamingScreen_CreateWaldaDadIcon(void)
{
    u8 spriteId;

    spriteId = CreateObjectGraphicsSprite(OBJ_EVENT_GFX_MAN_1, SpriteCallbackDummy, 56, 37, 0);
    gSprites[spriteId].oam.priority = 3;
    StartSpriteAnim(&gSprites[spriteId], ANIM_STD_GO_SOUTH);
}
```





# Options menu

## Setup tasks (via the initial main callback-2)

### State 0

```c
SetVBlankCallback(NULL);
```

### State 1

```c
//
// Reset VRAM.
//
DmaClearLarge16(3, (void *)(VRAM), VRAM_SIZE, 0x1000);
DmaClear32(3, OAM, OAM_SIZE);
DmaClear16(3, PLTT, PLTT_SIZE);
SetGpuReg(REG_OFFSET_DISPCNT, 0);
ResetBgsAndClearDma3BusyFlags(0);

//
// Initialize the backgrounds and windows used by this menu.
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
// Set up the special effect used to show the selected option: we darken the screen 
// overtop the options panel's content box, with a "window" (cutout) for the current 
// option row so that it isn't darkened.
//
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
```

The `WIN0H`, `WIN0V`, `WININ`, and `WINOUT` LCD I/O registers define "windowing" effects. These aren't related to Game Freak's "window" UI and drawing system, but rather use the term in a similar sense to GPU programming. They split the screen into four regions, and the background layers, object layer, and color special effects can be individually disabled for each region. The constants being used to manipulate these are defined in `gba/io_reg.h`.

* The LCD I/O register `DISPCNT` controls whether Window 0, Window 1, and the Object Window regions are enabled. If so, then the "outside of all windows" region is also automatically enabled.

* `WIN0H` and `WIN1H` define the horizontal dimensions of the two available windows. The high bits define the left edge; the low bits define the right edge plus 1. If the right edge exceeds 240px, the width of the screen, then it's clamped. If the edges are mismatched, then the right edge is forced to 240px.

* `WIN0V` and `WIN1V` define the vertical dimensions of the two available windows. They work the same as the horizontal dimensions, except that the limit is instead 160px, the height of the screen.

* `WININ` defines which layers are enabled inside of the two windows; there are three bits for each window, for a total of six bits. For example, `WININ_WIN0_BG0` declares background layer 0 as being visible inside of Window 0, and `WININ_WIN0_OBJ` declares the object layer as being visible inside of Window 0.

* `WINOUT` defines which layers are enabled outside of all windows, and which layers are enabled within the Object Window. Above, we enable both background layers outside of all windows, and we enable the color effect (managed by the `BLDCNT` and similar registers listed below) outside of them too (`WINOUT_WIN01_CLR`; that's "win 01" as in "outside of both windows").

The LCD I/O register `BLDCNT` controls blending effects for background layers. Here, `BLDCNT` is set to modify background layer 0 (`BLDCNT_TGT_BG0`) with a brightness decrease effect.

The LCD I/O register `BLDALPHA` controls alpha blending. The first target layer selected in `BLDCNT` is the "top" layer; the second target is the "bottom" layer. If only one target is selected, then `BLDALPHA` is not applied. (`BLDCNT` can also be used in conjunction with `BLDALPHA` to enable blending of the entire object layer as a single unit, but Game Freak doesn't do this here. Individual objects can be marked as semi-transparent in OAM, and if so, they're always used as the "top" layer wherever they are displayed.)

The LCD I/O register `BLDY` controls the intensity of `BLDCNT`'s brightness increase and decrease effects, when they're enabled. It's a four-bit value, so I believe the traditional [0,255] RGB value for the change would be `(value * 16) - 1`.

This is the options window, so later on, during its operation, we'll highlight an option with the following code:

```c
//
// Given:
// #define WIN_RANGE(a, b) (((a) << 8) | (b))
// #define WIN_RANGE2(a, b) ((b) | ((a) << 8))
//

SetGpuReg(REG_OFFSET_WIN0H, WIN_RANGE(16, DISPLAY_WIDTH - 16));
SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(index * 16 + 40, index * 16 + 56));
```

The sum effect of the values listed above is as follows:

* Set the current Color Effect to darken affected pixels in background layer 0 by 63.
* Enable background layer 0 and the object layer for the Window 1 region.
* Enable background layers 0 and 1, and the current Color Effect, for the Outside-of-Windows region.
* Configure Window 0 to span the width from 16 to `DISPLAY_WIDTH - 16`, and the height from `index * 16 + 40` to `index * 16 + 56`.

Notably, we've defined the options list area (the UI-window) to render on background layer 0 and to be 104x56px large, beginning at 8x20px. The menu header renders on background layer 1. All UI-window borders are drawn on background layer 1 via calls to `FillBgTilemapBufferRect`. The end result of these effects is that the options list content-box gets drawn on a darkened background layer, and Window 0 acts as a cutout for the darken effect.

### State 2

Reset on-screen visual effects, reset sprites and their palettes, and reset task state. All tasks are forcibly aborted and their data is cleared. Tasks aren't given any opportunity to perform teardown or cleanup operations, including for any memory allocations they may have made or may otherwise be responsible for freeing.

```c
ResetPaletteFade();
ScanlineEffect_Stop();
ResetTasks();
ResetSpriteData();
```

### State 3

Unique to the Options window. Loads the tiles for the player's chosen window frame.

```c
LoadBgTiles(1, GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->tiles, 0x120, 0x1A2);
```

### State 4

Loads two of the palettes used for the menu. The options menu palette loads to background palette ID 0, and the palette for the player's chosen window frame loads to background palette ID 7.

```c
LoadPalette(sOptionMenuBg_Pal, BG_PLTT_ID(0), sizeof(sOptionMenuBg_Pal));
LoadPalette(GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->pal, BG_PLTT_ID(7), PLTT_SIZE_4BPP);
```

### State 5

Loads the palette for options menu text to background palette ID 1.

```c
LoadPalette(sOptionMenuText_Pal, BG_PLTT_ID(1), sizeof(sOptionMenuText_Pal));
```

### State 6

Draws the content-box for the header window, which just shows the menu title.

```c
PutWindowTilemap(WIN_HEADER);
DrawHeaderText();
```

The `PutWindowTilemap` function performs an initial write for the window's tilemap.

### State 7

No-op. May have been used in vanilla.

### State 8

Draws the content-box for the options window and the option names.

```c
PutWindowTilemap(WIN_OPTIONS);
DrawOptionMenuTexts(0, TRUE);
```

### State 9

Draws the border-box for all windows.

```c
DrawBgWindowFrames();
```

### State 10

Creates and configures the "fade in" task for the menu; the task itself doesn't caues the fade-in, but rather just waits for it to complete before transforming itself into the "process input" task (by changing its own callback pointer). My code, here, also sets up the menu state and draws the current option values.

```c
u8 taskId = CreateTask(Task_OptionMenuFadeIn, 0);
// ...
```

### State 11

Begins a palette fade to actually trigger the fade-in.

```c
BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
SetVBlankCallback(VBlankCB);
SetMainCallback2(MainCB2);
```

The VBlank callback here is a very simple one:

```c
LoadOam();
ProcessSpriteCopyRequests();
TransferPlttBuffer();
```

The main callback-2 is similarly very simple:

```c
RunTasks();
AnimateSprites();
BuildOamBuffer();
UpdatePaletteFade();
```