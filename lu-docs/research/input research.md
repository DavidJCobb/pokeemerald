
It'd be useful to map out all of the player controls so that we know what we have the leeway to add, and so we can add things without potentially running into input conflicts. To give one example, I forgot the "L=A" option even existed until I started looking into input.

The game's current controls, sorted by context and ordered by descending priority/chronology unless otherwise stated, are as follows:

* Global
** [`main.c`/`ReadKeys`] **L:** Aliased to **A** depending on user options.
* Menu context
** *[Order unknown.]**
** **A:** Commit
** **B:** Back/Cancel
** **D-Pad:** Cursor movement
** **L:** Menu-specific uses accessed via `GetLRKeysPressed` and `GetLRKeysPressedAndHeld`, conditioned to the "Button Mode" option's "LR" value.
*** Some menus also access this directly:
**** `pokedex.c` maps `L_BUTTON` and `R_BUTTON` to the same functions as `DPAD_LEFT` and `DPAD_RIGHT`, if the "Button Mode" option enables L/R.
**** `pokedex_area_screen.c` maps `DPAD_RIGHT` to the same function as `R_BUTTON`, if the "Button Mode" option enables L/R.
**** `pokemon_storage_system.c` maps `L_BUTTON` and `R_BUTTON` to the "input scroll left" and "input scroll right" actions defined internally, if the "Button Mode" option enables L/R.
**** `union_room_chat.c` checks `R_BUTTON` **even if the "Button Mode" option doesn't enable L/R.**
**** `slot_machine.c` maps `R_BUTTON` to making the maximum possible bet, **even if the "Button Mode" option doesn't enable L/R.**
** [`ListMenu_ProcessInput`] **L:** Intended to scroll multiple items in some listviews when held down. Broken by the implementation of the "L=A" button mode; see comments in `main.c` `ReadKeys`.
** [`ListMenu_ProcessInput`] **R:** Intended to scroll multiple items in some listviews when held down.
* Overworld context
** [`ProcessPlayerFieldInput`] **B:** Resurface when underwater
*** *[Only when not at Mach Bike top speed (checked previously, in `FieldGetPlayerInput`).]*
** [`ProcessPlayerFieldInput`] **Start:** Open pause menu
*** *[Only when not at Mach Bike top speed (checked previously, in `FieldGetPlayerInput`).]*
** [`ProcessPlayerFieldInput`] **A:** Interact
*** *[Only when not at Mach Bike top speed (checked previously, in `FieldGetPlayerInput`).]*
** **D-Pad:** Movement
*** *[If we want to implement Overworld button combos using the D-Pad, outside of a menu context, then early in ProcessPlayerFieldInput (in a branch returning `TRUE`) would be the place to do that.]*
** [`ProcessPlayerFieldInput`] **Select:** Use hotkeyed item
*** *[Only when not at Mach Bike top speed (checked previously, in `FieldGetPlayerInput`).]*
* Link system
** *[Order unknown.]*
** [`LinkTestProcessKeyInput`] **A:** Advance link state.
** [`LinkTestProcessKeyInput`] **B:** Unknown.
** [`LinkTestProcessKeyInput`] **L:** Unknown.
** [`LinkTestProcessKeyInput`] **Start:** Suppress link error messages.
** [`LinkTestProcessKeyInput`] **R:** Unknown.
** [`LinkTestProcessKeyInput`] **Select:** Unknown.
* Pokemon Storage System
** *[This menu system defines an internal set of input handlers, and then converts inputs received from `main.c` to these input handler codes. Ordering is knowable but I'd have to look at the overall system more closely in order to know how best to express this in natural language.]*
*** Inputs are mapped in the following functions:
**** `InBoxInput_Normal`
**** `InBoxInput_SelectingMultiple`
**** `InBoxInput_MovingMultiple`
**** `HandleInput_InParty`
**** `HandleInput_OnBox` (when cursor is on box title)
**** `HandleInput_OnButtons`
** **ToggleCursorAutoAction():** Mapped to the select button, during multiple input handlers
** **INPUT_MOVE_CURSOR:** D-Pad. Start button (moves between box title and box contents).
** **INPUT_CLOSE_BOX:** Also used to return from the Party to the Box, via the A-button on "close."
** **INPUT_SHOW_PARTY:** 
** **INPUT_HIDE_PARTY:** 
** **INPUT_BOX_OPTIONS:** A-button when cursor is on box title.
** **INPUT_IN_MENU:** 
** **INPUT_SCROLL_RIGHT:** R-button if L/R is enabled. D-Pad Right when cursor is on box title.
** **INPUT_SCROLL_LEFT:** L-button if L/R is enabled. D-Pad Left when cursor is on box title.
** **INPUT_DEPOSIT:** A-button on "Deposit" menu item.
** **INPUT_WITHDRAW:** A-button on "Withdraw" menu item.
** **INPUT_MOVE_MON:** A-button on "Move" menu item.
** **INPUT_SHIFT_MON:** A-button on "Shift" menu item.
** **INPUT_PLACE_MON:** A-button on "Place" menu item.
** **INPUT_TAKE_ITEM:** A-button on "Take Item" menu item.
** **INPUT_GIVE_ITEM:** A-button on "Give Item" menu item.
** **INPUT_SWITCH_ITEMS:** A-button on "Switch Items" menu item.
** **INPUT_PRESSED_B:** B-button if not caught by anything else. Includes returning from Party to Box.
** **INPUT_MULTIMOVE_START:** A-button when multi-select is active and you have no selections.
** **INPUT_MULTIMOVE_CHANGE_SELECTION:** A + D-Pad.
** **INPUT_MULTIMOVE_SINGLE:** Unknown.
** **INPUT_MULTIMOVE_GRAB_SELECTION:** Unknown.
** **INPUT_MULTIMOVE_UNABLE:** *Generic code for disallowed inputs.*
** **INPUT_MULTIMOVE_MOVE_MONS:** D-Pad.
** **INPUT_MULTIMOVE_PLACE_MONS:** A-button.