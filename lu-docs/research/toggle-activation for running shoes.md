
**This feature has been implemented. Some differences exist between the implementation and the plans described below, as real-world testing uncovered edge-cases that needed fixing.**

# Overworld

Call stack for player movement on the overworld:

* `field_player_avatar.c`: **PlayerStep**
** HideShowWarpArrow
** Bike_TryAcroBikeHistoryUpdate
** TryInterruptObjectEventSpecialAnim
** **npc_clear_strange_bits(playerObjEvent)**
   Clears a few state flags on the passed-in object, and clears `PLAYER_AVATAR_FLAG_DASH` from `gPlayerAvatar.flags` no matter what object event is passed in.
** DoPlayerAvatarTransition()
** **TryDoMetatileBehaviorForcedMovement()**
   Looks up the appropriate forced movement type for the player's current metatile, and invokes the handler for that movement type. These handlers return `TRUE` if they force the player to move, or `FALSE` if they don't. This is conditional in part because "none" is a forced movement type, and in part because muddy slopes conditionally force movement (i.e. you slip if you're not on the Mach Bike or haven't reached top speed with it).
** MovePlayerAvatarUsingKeypadInput(direction, newKeys, heldKeys)
*** Conditionally calls either MovePlayerOnBike or MovePlayerNotOnBike.
*** **MovePlayerNotOnBike**
    Gets the player's running state (NOT_MOVING/TURN_DIRECTION/MOVING) and indexes into a list of handlers, `sPlayerNotOnBikeFuncs`, invoking the appropriate one with `direction` and `heldKeys` as arguments.
**** **PlayerNotOnBikeMoving**
     Handler for moving from one tile to another when not on a bike.
***** PlayerJumpLedge(direction)
***** PlayerNotOnBikeCollideWithFarawayIslandMew(direction)
***** PlayerNotOnBikeCollide(direction)
***** **PlayerWalkFast(direction)**
      Here, used for surfing movement at the same speed as running.
***** **PlayerRun(direction); gPlayerAvatar.flags |= PLAYER_AVATAR_FLAG_DASH;**
      Executed if the following conditions are met: the player is not underwater; `heldKeys` includes `B_BUTTON`; `FLAG_SYS_B_DASH` is set; and IsRunningDisallowed returns 0 for the current metatile behavior. The PlayerRun function sets the player to use run animations and movement, using the same underlying system as is used for NPCs' scripted movement.
** PlayerAllowForcedMovementIfMovingSameDirection()

Per the above call stack, the player can run if the following conditions are met:

* `PlayerStep`: `gPlayerAvatar.preventStep == FALSE`.
* `PlayerStep`: `TryInterruptObjectEventSpecialAnim(playerObjEvent, direction) == 0`.
* `PlayerStep`: A forced movement does not occur.
* `MovePlayerAvatarUsingKeypadInput`: The player is not on the Mach Bike or the Acro Bike.
* `MovePlayerNotOnBike`: The player is, or is going to be, `MOVING`, per `CheckMovementInputNotOnBike(direction)`. This means that either they're already running, or they're standing still but their chosen movement direction matches their facing direction.
* `PlayerNotOnBikeMoving` early return: The player is not colliding.
* `PlayerNotOnBikeMoving` early return: The player is not surfing.
* `PlayerNotOnBikeMoving`: The player is not underwater.
* `PlayerNotOnBikeMoving`: The player is holding the B button.
* `PlayerNotOnBikeMoving`: `FLAG_SYS_B_DASH` is set. (The player has received the Running Shoes.)
* `PlayerNotOnBikeMoving`: Running is allowed for the current metatile behavior.

The `PLAYER_AVATAR_FLAG_DASH` flag is checked in a few places:

* `bike.c` `GetPlayerSpeed` returns `PLAYER_SPEED_FAST` if the player is surfing, or if they're on foot with `PLAYER_AVATAR_FLAG_DASH`. This function doesn't seem to influence player movement, though; it's used as a signal to other systems, like muddy slopes.
* `event_object_movement.c` `ObjectEventIsTrainerAndCloseToPlayer` returns true if the player is running near the object event, if it's a trainer (even a buried one), and if the player's destination tile is within the trainer's vision distance.

But again, actual movement occurs as a result of `PlayerRun` calling through `PlayerSetAnimId` to `ObjectEventSetHeldMovement`. (The middle function also sets the player's "copyable movement," used by NPCs that follow or mirror the player. Because NPCs can't run, the copyable movement for running is walking.) The `ObjectEventSetHeldMovement` function sets the player-object-event's movement parameters, again using the same system as walking NPCs.

Finally, let's zoom out one level. How does input reach `PlayerStep`?

* **DoCB1_Overworld(u16 newKeys, u16 heldKeys)**
  The top-level callback for player input and movement within the overworld.
** **FieldClearPlayerInput(&inputStruct);**
   Clears a struct containing player input state.
** **FieldGetPlayerInput(&inputStruct, newKeys, heldKeys);**
   Updates that same struct. Defined in `field_control_avatar.c`. Notably, this function only sets `inputStruct.pressedBButton` to `TRUE` if the player is not between tiles and if `GetPlayerSpeed()` is not `PLAYER_SPEED_FASTEST`.
** ArePlayerFieldControlsLocked
** **ProcessPlayerFieldInput(&inputStruct)**
   Allows systems other than movement to intercept and handle player input. If this function returns 1, then player field controls are locked, the map name popup window is hidden, and `PlayerStep` is not called.
** **PlayerStep(inputStruct.dpadDirection, newKeys, heldKeys)**
   See above.

We lose access to more and more state as we dive deeper into input-related code. Player steps only have access to the player's D-pad direction (as the intended movement direction), the newly-pressed keys, and the held keys.

## Considerations for a toggle

* Pressing B while underwater is how you trigger the confirmation prompt to return to the surface. We must only allow the player to toggle running while they are on foot and above water.
* We'd want to override the check in `PlayerNotOnBikeMoving` to check whether the "player has toggled running on" flag is set. However, where should we set and clear the flag? Where should we handle that initial B-press?
* If the player is forced into a scripted movement, we'll need to either clear or ignore our "running is active" flag.
* Pokemon Inclement Emerald allows players to set Auto-Run by pressing the "R" button. This then inverts the behavior of holding "B:" you run automatically by default, and holding B allows you to walk.
** The vanilla game doesn't check the R-button in its standard input handler, so the overworld can't easily see it. Specific menus query hardware state directly via `JOY_NEW` and `JOY_HELD` for the R-button.
*** It's worth noting that Emerald offers an "L=A" option wherein the main input handler treats the L-button as an alias of the A button; this is the only reason the game ever checks the L-button, and it influences the input vars that are handed down to the overworld.
** The problem with implementing this is that we can't then use the R-button for other purposes, e.g. opening an item hotkey menu, unless we maybe map that to "Hold R for X many frames" whenever R is mapped to inverting input.
** Maybe it'd be better to forego offering the Inclement Emerald option for now. We can still lay the groundwork for it, but eventually investigate implementing input remapping a la a modern video game.

### Proposals, then:

* Rename `FieldInput::pressedAButton` to `FieldInput::fieldButtonInteract`, since that better reflects its purpose. The flag is only checked for two reasons: to attempt an interaction on whatever tile is in front of the player (be it interactable terrain or a placed event); and to attempt interaction with a dive spot. (Both calls are made from `ProcessPlayerFieldInput`.)

* Rename `FieldInput::pressedBButton` to `FieldInput::fieldButtonResurface` since that better reflects its purpose. The flag is only checked to attempt to resurface (via a call from `ProcessPlayerFieldInput` to `TrySetupDiveEmergeScript`).

* Implement the following content:
```c
enum {
   LU_OVERWORLD_RUN_OPTION_VANILLA,
   LU_OVERWORLD_RUN_OPTION_TOGGLE,   // "B" toggles running; "hold B" does nothing
   LU_OVERWORLD_RUN_OPTION_INVERT,   // planned but unimplemented: "R" toggles running; "hold B" does the opposite
};

u8   lu_GetOverworldRunOption(void);
void lu_SetOverworldRunOption(u8);

bool8 lu_GetOverworldRunExtBehaviorActive(void);
void  lu_ToggleOverworldRunExtBehaviorActive(void); // on B-press or on R-press, depending on option

bool8 lu_PlayerShouldRun(u16 heldKeys);
```

* Replace the following code in `ProcessPlayerFieldInput`
```c
   if (input->pressedBButton && TrySetupDiveEmergeScript() == TRUE)
      return TRUE;
```
  with:
```c
   if (input->pressedBButton) {
      if (TrySetupDiveEmergeScript() == TRUE) {
         return TRUE;
      }
      lu_TogglePlayerOverworldRunning();
   }
```

* Modify code in `PlayerNotOnBikeMoving`: replace the `(heldKeys & B_BUTTON)` check with a call to `lu_PlayerShouldRun(heldKeys)`.


# Options menu

We'll also need to add an entry to the options menu for this. That's more complicated than it seems, however, for a few reasons. The vanilla options menu displays exactly enough menu items to fill the screen, and all of an option's possible values are displayed on-screen at once -- with the sole exception being the "Frame" option.

When the menu is first opened, the `DrawHeaderText()` function is used to draw the menu header, and the `DrawOptionMenuTexts()` function is used to draw the option names, which are stored in array `sOptionMenuItemsNames`. These things are never redrawn again for the rest of the menu being open, and there is no functionality already in place for being able to scroll the menu (i.e. it is assumed that all options in the list will fit on-screen).

Once the menu is operable, the top-level `Task_OptionMenuProcessInput` function handles exiting the menu (saving your changes) and moving the cursor vertically. For all other inputs, it does a switch-case on the currently highlighted option. Each option has a "process input" function and a "draw choices" function: the former is first invoked, and then the latter is invoked if and only if the option's value has changed.

The options menu works via the game's "task" system, by the way, and the menu state is stored as task-local data. Tasks have 16 uint16_t fields available for use, of which one is used to track the highlighted menu option; this leaves room to store 15 option selections in task data, and the vanilla menu has 6 options.

At minimum, then, the following changes are needed:

* `DrawOptionMenuTexts()` needs to be modified so that it clears the areas it's drawing to.
* We need a state variable (we can use a task-state field for now) to track the current scroll offset.
* For the D-Pad Up branch in `Task_OptionMenuProcessInput`, we need to scroll up if the cursor is above the halfway point on-screen and the scroll offset is greater than zero.
** Scrolling should retrigger a redrawing of the menu option names and of all options choices.
** When scrolling, remember that `HighlightOptionMenuItem` takes a screen-relative option index. They basically generate a tiled graphic that darkens all but one row, and mess with GPU registers to move it.
* For the D-Pad Down branch in `Task_OptionMenuProcessInput`, we need to scroll down if the cursor is below the halfway point on-screen and the scroll offset is less than (option count minus on-screen row count).
* All of the "draw choices" functions will need to be updated to receive a new argument: the Y-position at which they should draw their text.
** Actually, we'll need to make a few more changes, too. The current beahvior is to just change the text color, so none of the "draw choices" functions ever actually clear out the area they're about to draw to. I'll need to see how easy or difficult it'd be to have them do that. (You might be thinking: wait, wouldn't the "Frame" option have to clear text, since it shows text in one spot? Yes, but that happens as a side effect of displaying the text: when you print text overtop other text, the stuff beneath is cleared. We need to handle the places where old text exists but new text isn't being printed.)
*** `FillWindowPixelRect` in `gflib/window.h` (just `#include window.h`; input paths are hardcoded in this build system) takes a window ID, window-relative X and Y offsets (in pixels), fill value, and size (in pixels) to fill, along with the fill value, of course. The fill value will be `PIXEL_FILL(somePaletteIndex)`; try palette index 1 for now.
* We need to add branches and strings for our "running" option, with values "HOLD" and "TOGGLE".

However, the following larger-scale changes would make the options menu more robust:

* Currently, there's an array of option names, with hardcoded branches for each option. However, C supports function pointers. We could instead replace this with an array of structs, each defining the option's name and the functions for checking input and drawing choices. This would allow us to replace several branches with indexed behavior, making the code simpler, and it'd make it easier to add even more options (up to our limit of 14, given 2 task fields for menu state).
* It'd be nice if an option could specify that it's just a boolean, in which case we could draw its "choices" as a checkbox. We could have "Battle Scene" be a checkbox, and we could rename "Sound" to "Stereo Audio" and make it a checkbox.
* If we overhaul the "draw choices" system entirely, then we could make it possible to add options with more values, and values with longer names. Games like Halo 3 only display the currently selected value, with arrows to indicate that you can move left or right to change it. If we switch to that display, then each option can just define a list of value texts; we can fully genericize drawing the options' values.
* At the upper limit of possibility, it'd be nice if the bottom of the screen could be used to display a keybind list (D-Pad, A to cycle, B to cancel, etc.) and above that, a one-line description of the currently highlighted option or its value. We could give ourselves more screen real-state if we ditch the inset windows for a more custom UI (think of that used by FR/LG's Teachy TV).