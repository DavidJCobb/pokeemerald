
# Battle controllers

A <dfn>battle controller</dfn> is a trainer participating in an ongoing battle. Battle controllers: are placed in charge of [battler positions](#battler-position) as appropriate; receive *commands*, such as "choose an action from the fight/PKMN/bag/run menu" or "play a Pokemon's fainting cry animation;" and perform actions in response to those commands. (Those actions can potentially include emitting a command in response.) Battle controllers are a key element in the abstractions that allow the same battle engine to handle real-time singleplayer battles, linked multiplayer battles, cutscene battles like Wally's capture of Ralts, and recorded battles. The battle engine can simply demand input from whoever or whatever is at the helm, and it doesn't have to care how those inputs are supplied &mdash; whether they're coming from the player, from an NPC, from over a multiplayer link, or what. The battle engine just ferries command data to where it needs to go and invokes the battle controller function pointer for each [battler](./battle%20concepts#battler-id) on the field.

Because battle controllers are tied to positions on the battlefield, the abstraction may not work the way you expect. For example, you might expect most player-facing GUI to be displayed by the local-player controller. However, some GUI elements are tied to battlefield positions, such as making a Pokemon's sprite flash when it takes damage, playing a Pokemon's cry when it faints, or even showing a trainer's party summary (the Poke Balls you see telling you how many Pokemon an opponent has); and so these would actually be performed by the NPC-opponent or link-opponent controllers.

Similarly, some battle controller commands may not immediately be intuitive. There are commands which instruct a battle controller to load and display a given battler's sprite, for example. Why are these handled by the controller, and not by the core battle engine? The Safari Zone is one reason. When facing off against Wild Pokemon in the Safari Zone, the player's half of the battlefield uses a special Safari Zone controller. Under the hood, the player sends Pokemon out to battle as normal, but the Safari Zone controller refuses to load or display any of the associated graphics, in order to create the illusion that the player is facing off against the Wild Pokemon alone.

See, as far as the battle engine is concerned, you are *always* using your Pokemon to battle some opposing Pokemon. If your battle controller chooses to hide that fact from you, while also only ever allowing you to select actions specific to the Safari Zone, that's not the battle system's business. Similarly, if the Wild Pokemon's battle controller checks the current battle type, sees that it's a Safari Zone battle, and chooses never to actually attack your Pokemon (which are there even if you can't see them), then that's all fine and dandy as far as the battle system is concerned, and the system doesn't care why those decisions are being made.


## Vanilla battle controllers

The following battle controllers exist:

* The player
* The player, as a Safari Zone combatant
* The player's NPC ally in a Multi Battle
* The player's link ally in a Link Multi Battle
* Wally, acting instead of the player
* The opponent NPC
* The opponent player
* The player's past actions in a recorded battle
* The player's past link opponent in a recorded battle[^no-recorded-npc-controller]

[^no-recorded-npc-controller]: There is no controller for recorded NPC opponents or allies; the NPC opponent controller is used. The NPC AI may be deterministic, or it may take recorded battle data into account on its own without needing an entire separate controller.

## Control flow

The various `BtlController_Emit<Whatever>` functions in [`battle_controllers.h`](/include/battle_controllers.h)/[`.c`](/src/battle_controllers.c) use `sBattleBuffersTransferData` to build commands and then send them via `PrepareBufferDataTransfer`. When these commands are eventually received, they are then dispatched to the active battle controller, which must obey them. How are they dispatched?

Each [battler ID](./battle%20concepts.md#battler-id) has an associated callback function pointer for battle controllers. On every frame, `BattleMainCB1` invokes these functions for every battler, setting `gActiveBattler` to the battler in question.

Generally, when a controller is idle, its callback pointer is set to a function with a name like `<ThisControllerName>BufferRunCommand()`. This function checks whether the `gBattleControllerExecFlags` flag is set for `gActiveBattler`. If the flag is set, then `gBattleBufferA[gActiveBattler][0]` is treated as a [command](#commands) to run, and the appropriate function is called to handle that command. This function may be a [latent function](./battle%20concepts.md#latent), doing its work over multiple frames by overwriting the battler's callback pointer. When the command is finally handled, the controller will call `<ThisControllerName>BufferExecCompleted()`, which will set the battler's callback pointer back to `<ThisControllerName>BufferRunCommand`, before either transmitting data over the link cable (if the current battle is a Link Battle) or clearing the `gBattleControllerExecFlags` flag for `gActiveBattler`.

## Commands

Battle controllers need to be able to handle the following commands. Most commands are "inbound:" they are emitted by various parts of the battle engine, and the controller is expected to react to them in some way. Some commands are "outbound:" the controller itself emits them in response to commands it has received.

(Perhaps "message" would've been a better word than "command.")

| Command | Direction | Description |
| :- | :-: | :- |
| CONTROLLER_GETMONDATA           | Inbound | |
| CONTROLLER_GETRAWMONDATA        | Inbound | |
| CONTROLLER_SETMONDATA           | Inbound | |
| CONTROLLER_SETRAWMONDATA        | Inbound | |
| CONTROLLER_LOADMONSPRITE        | Inbound | |
| CONTROLLER_SWITCHINANIM         | Inbound | |
| CONTROLLER_RETURNMONTOBALL      | Inbound | Play the animations for returning `gActiveBattler` to its Poke Ball, as part of switching it out of battle. |
| CONTROLLER_DRAWTRAINERPIC       | Inbound | |
| CONTROLLER_TRAINERSLIDE         | Inbound | |
| CONTROLLER_TRAINERSLIDEBACK     | Inbound | |
| [CONTROLLER_FAINTANIMATION](#CONTROLLER_FAINTANIMATION) | Inbound | Play a fainting animation for `gActiveBattler` and wait for the animation to complete. If this is the local player controller, then now would be a good time to update the low HP sound as well. |
| CONTROLLER_PALETTEFADE          | Inbound | |
| CONTROLLER_SUCCESSBALLTHROWANIM | Inbound | Play the animation for throwing a Poke Ball from `gActiveBattler`'s position on the battlefield, with ball throw case ID `BALL_3_SHAKES_SUCCESS`. For the local player controller, the ball is assumed to have been thrown at `B_POSITION_OPPONENT_LEFT`. |
| CONTROLLER_BALLTHROWANIM        | Inbound | Play the animation for throwing a Poke Ball from `gActiveBattler`'s position on the battlefield. The ball throw case ID (indicating the number of shakes) is in `gBattleBufferA[gActiveBattler][1]` and is one of the values in the `BALL_` enum in [`battle_controllers.h`](/include/battle_controllers.h). For the local player controller, the ball is assumed to have been thrown at `B_POSITION_OPPONENT_LEFT`. |
| [CONTROLLER_PAUSE](#CONTROLLER_PAUSE) | Inbound | Unused and probably broken. Seemingly intended to briefly pause the battle controller's functionality. |
| CONTROLLER_MOVEANIMATION         | Inbound | |
| CONTROLLER_PRINTSTRING           | Inbound | Display text to the player. `*(u16*)&gBattleBufferA[gActiveBattler][2]` is a battle string ID. Controllers for the player and their ally should generally display the string and then wait until the text printer is finished. |
| CONTROLLER_PRINTSTRINGPLAYERONLY | Inbound | Similar to `CONTROLLER_PRINTSTRING`, except the string should only be displayed for the player: if `gActiveBattler` is not on `B_SIDE_PLAYER`, then do nothing. |
| [CONTROLLER_CHOOSEACTION](#CONTROLLER_CHOOSEACTION) | Inbound | The controller should decide what kind of action they're going to perform &mdash; not the specifics; just what kind. In typical battles, this is the top-level menu (Fight/Pokemon/Bag/Run), but other important actions are available here as well. |
| CONTROLLER_YESNOBOX | Inbound | The controller should respond to a yes/no dialog box. To make your choice, call `BtlController_EmitTwoReturnValues(BUFFER_B, choice, 0)`, where `choice` is `0xD` for yes or `0xE` for no. |
| [CONTROLLER_CHOOSEMOVE](#CONTROLLER_CHOOSEMOVE) | Inbound | The controller responded to `CHOOSEACTION` by deciding to fight, so now it has to either pick a move to use and a target to use it on, or back out to the action menu and pick an action again. |
| [CONTROLLER_OPENBAG](#CONTROLLER_OPENBAG) | Inbound | The controller responded to `CHOOSEACTION` by deciding to use an item, so now it has to either pick an item, or back out to the action menu and pick an action again. |
| CONTROLLER_CHOOSEPOKEMON            | Inbound | |
| CONTROLLER_23                       | Unknown | Unused. |
| [CONTROLLER_HEALTHBARUPDATE](#CONTROLLER_HEALTHBARUPDATE) | Inbound | The controller is being told to update the state of a battler's on-screen health bar. |
| [CONTROLLER_EXPUPDATE](#CONTROLLER_EXPUPDATE)             | Inbound | The controller is being told to award EXP to a Pokemon it owns (which may or may not be on the battlefield right now). |
| CONTROLLER_STATUSICONUPDATE         | Inbound | |
| CONTROLLER_STATUSANIMATION          | Inbound | |
| CONTROLLER_STATUSXOR                | Inbound | |
| CONTROLLER_DATATRANSFER             | Outbound | This command is emitted *by* controllers, not *to* them, and is how a controller can respond to a command by offering arbitrarily-sized data. |
| CONTROLLER_DMA3TRANSFER             | Unknown | Unused. |
| [CONTROLLER_PLAYBGM](#CONTROLLER_PLAYBGM) | Inbound | Unused, and emitting it is broken. The controller is being asked to set the current background music. |
| CONTROLLER_32                       | Unknown | Unused. |
| CONTROLLER_TWORETURNVALUES          | Outbound | This command is emitted *by* controllers, not *to* them, and is how a controller can respond to a command by offering two return values. |
| CONTROLLER_CHOSENMONRETURNVALUE     | Outbound | This command is emitted *by* controllers, not *to* them, and is how a controller can respond to a command by offering a chosen Pokemon. |
| CONTROLLER_ONERETURNVALUE           | Outbound | This command is emitted *by* controllers, not *to* them, and is how a controller can respond to a command by offering a single return value. |
| CONTROLLER_ONERETURNVALUE_DUPLICATE | Outbound | This command is emitted *by* controllers, not *to* them, and is how a controller can respond to a command by offering a single return value. |
| CONTROLLER_CLEARUNKVAR              | Unknown | Unused. |
| CONTROLLER_SETUNKVAR                | Unknown | Unused. |
| CONTROLLER_CLEARUNKFLAG             | Unknown | Unused. |
| CONTROLLER_TOGGLEUNKFLAG            | Unknown | Unused. |
| CONTROLLER_HITANIMATION             | Inbound | The controller should play the animation for `gActiveBattler` being hit (e.g. flashing its sprite and calling `DoHitAnimHealthboxEffect(gActiveBattler)`, if the battler is not invisible). The controller should wait for the animation to complete before proceeding further. |
| [CONTROLLER_CANTSWITCH](#CONTROLLER_CANTSWITCH) | Inbound | A scrapped command that would've been used to tell the player during Double Battles, "You don't have any remaining Pokemon to replace your newly-fainted battler." |
| CONTROLLER_PLAYSE | Inbound | The controller is being asked to play a sound effect. The sound ID is the big-endian two-byte value at `&gBattleBufferA[gActiveBattler][1]`. The controller need not wait for the sound to complete. |
| [CONTROLLER_PLAYFANFAREORBGM](#CONTROLLER_PLAYFANFAREORBGM) | Inbound | The controller is being asked to play a fanfare or set the background music. |
| CONTROLLER_FAINTINGCRY | Inbound | The controller should play the fainting cry sound effect for `gActiveBattler`. It need not wait for the sound to complete. |
| [CONTROLLER_INTROSLIDE](#CONTROLLER_INTROSLIDE) | Inbound | The battle is starting, and the background art for this battle (depicting the terrain) should slide into view. |
| CONTROLLER_INTROTRAINERBALLTHROW     | Inbound | |
| CONTROLLER_DRAWPARTYSTATUSSUMMARY    | Inbound | |
| CONTROLLER_HIDEPARTYSTATUSSUMMARY    | Inbound | |
| CONTROLLER_ENDBOUNCE                 | Inbound | |
| CONTROLLER_SPRITEINVISIBILITY        | Inbound | The controller should update the invisibility state for `gActiveBattler`'s sprite, setting it to `gBattleBufferA[gActiveBattler][1]`. The controller should make the appropriate checks to verify that there's even a sprite there to update, first. |
| CONTROLLER_BATTLEANIMATION           | Inbound | |
| CONTROLLER_LINKSTANDBYMSG            | Inbound | |
| CONTROLLER_RESETACTIONMOVESELECTION  | Inbound | |
| CONTROLLER_ENDLINKBATTLE             | Inbound | |
| CONTROLLER_TERMINATOR_NOP            | N/A | A no-op byte used to reset command buffers when they're not in use. Controllers deliberately do nothing in response to this command. |

### `CONTROLLER_FAINTANIMATION`
The controller is being directed to play the fainting animation for a battler, and wait for the animation to complete.

The controller should handle the case of a Pokemon fainting while behind Substitute, and it should free the battler's sprite after the fainting animation completes.

### `CONTROLLER_PAUSE`
An unused controller command seemingly intended to make the controller pause for a few frames. It's never emitted, only the local player controller responds to it, and said controller seems to handle it incorrectly.

`gBattleBufferA[gActiveBattler][1]` is a delay value, but the only controller that actually heeds it is the local player controller. That controller doesn't delay for <var>n</var> many frames, as you would expect, but rather just does `while (timer != 0) --timer;`. (So... what, it'd pause for <var>n</var> *clock cycles*?? That can't be intentional.)

### `CONTROLLER_CHOOSEACTION`
The controller must choose an option from the top-level menu (Fight/Pokemon/Bag/Run). In a typical battle, this will be a decision as to whether to use a move, use an item, switch Pokemon, or attempt to flee. Note that the controller isn't yet telling the battle engine *which* move, item, or party member it wants to use.

To make your choice, call `BtlController_EmitTwoReturnValues(BUFFER_B, choice, 0)`. where `choice` is the appropriate `B_ACTION_` constant from [`battle.h`](/include/battle.h):

| Constant | Meaning |
| :- | :- |
| `B_ACTION_USE_MOVE` | Use a move (the "Fight" option in the player-facing top-level menu). After you give this response, you should expect to receive a `CONTROLLER_CHOOSEMOVE` command. |
| `B_ACTION_USE_ITEM` | Use an item (the "Bag" option in the player-facing top-level menu). |
| `B_ACTION_SWITCH` | Switch out this battler for a different party member. |
| `B_ACTION_RUN` | Attempt to flee. This action is used by both the player and by Wild Pokemon. |
| `B_ACTION_SAFARI_WATCH_CAREFULLY` | This is the "do nothing" action that Wild Pokemon perform in the Safari Zone. |
| `B_ACTION_SAFARI_BALL` | Throw a Safari Ball at a Wild Pokemon in the Safari Zone. |
| `B_ACTION_SAFARI_POKEBLOCK` | Throw a PokeBlock at a Wild Pokemon in the Safari Zone. |
| `B_ACTION_SAFARI_GO_NEAR` | Step closer to a Wild Pokemon in the Safari Zone. |
| `B_ACTION_SAFARI_RUN` | Flee from a Wild Pokemon in the Safari Zone. |
| `B_ACTION_WALLY_THROW` | Have Wally throw a Poke Ball at the opposing Wild Pokemon. |
| ~~`B_ACTION_EXEC_SCRIPT`~~ | Used internally by the battle engine, and never by any battle controllers. Further investigation required to determine if this is legal for battle controllers to use (i.e. link-safe, etc.). |
| ~~`B_ACTION_TRY_FINISH`~~ | Used internally by the battle engine and battle script engine, and never by any battle controllers. |
| ~~`B_ACTION_FINISHED`~~ | Used internally by the battle engine, and never by any battle controllers. Performed when a battler's chosen action is finished, and the next battler in the turn can perform their action. |
| `B_ACTION_CANCEL_PARTNER` | In a Double Battle (that isn't also a Multi Battle), when commanded to choose the action of your second Pokemon on the field, choose this action to cancel the selections you made for your first Pokemon on the field (so you can change your mind). |
| ~~`B_ACTION_NOTHING_FAINTED`~~ | Used internally by the battle engine, and never by any battle controllers. This action appears to be automatically selected for empty positions on the field (i.e. if no one's in a given spot, then No One does "nothing/fainted"). |
| ~~`B_ACTION_NONE`~~ | Used internally by the battle engine, and never by any battle controllers. When no action has yet been selected for a given battler, `gChosenActionByBattler[i]` is this value. |

### `CONTROLLER_CHOOSEMOVE`
The controller responded to `CHOOSEACTION` by deciding to fight, so now it has to pick a move to use and a target to use it on.

The `gBattleBufferA[gActiveBattler]` buffer is laid out as follows:

| Offset | Type | Description |
| -: | :-: | :- |
| 0 | `u8` | Command ID: `CONTROLLER_CHOOSEMOVE`. |
| 1 | `bool8` | Indicates that this battle is a Double Battle. |
| 2 | `bool8` | In single battles, indicates that the controller should not be allowed to choose the target of a `MOVE_TARGET_USER_OR_SELECTED` move (as they would normally be able to). Whether it indicates anything else, I don't know. |
| 3 | `u8` | unknown/padding |
| 4 | `ChooseMoveStruct` | The moves that the controller has available to choose from, along with their current and max PP, the species of the Pokemon for which they're choosing a move, and the types of that Pokemon. |

If it's a controller for the local player, you can allow them to reorder moves here; just be sure to update the `ChooseMoveStruct` associated with this command, the move data for `gBattleMons[gActiveBattler]`, and the persistent (non-battle) Pokemon data (i.e. `gPlayerParty[gBattlerPartyIndexes[gActiveBattler]]`). Doesn't look like there's any need to synchronize these sorts of changes over a multiplayer link, at least not immediately.

To choose a move to use, call `BtlController_EmitTwoReturnValues(BUFFER_B, 10, choice | (target << 8))`, where `choice` is the index of the move to use, and `target` is the [battler position](#battler-position) to attack. Alternatively, to back out of the move selection menu (as the player themselves can), call `BtlController_EmitTwoReturnValues(BUFFER_B, 10, 0xFFFF)`.

### `CONTROLLER_OPENBAG`
The controller responded to `CHOOSEACTION` by deciding to use an item, so now it has to pick an item to use and an owned or allied battler to use it on.

Choose an item (or PokeBlock) to use by calling `BtlController_EmitOneReturnValue(BUFFER_B, itemID)`. To back out of the item selection menu (as the player themselves can), use zero as the item ID.

Note that the battle engine doesn't appear to be fully responsible for *actually using* the item:

* When the local player chooses to use an item, the item use callback[^item-use-callback], if one exists, is invoked as part of the item selection process, before the item ID is sent to the battle system: the effects of the item are applied before the battle system ever gets a chance to know what's happened, and the battle system isn't specifically told what those effects were. This, it seems, is why items aren't usable in Link Battles: all item effects are designed to run locally, with no provisions made to let them synch.

  That said, there are some items that are specifically designed to integrate into the battle system exclusively, such as Poke Balls, Poke Dolls, Fluffy Tails, and so on. These are handled by `HandleAction_UseItem`, defined in [`battle_util.c`](/src/battle_util.c). That function relies on [`gLastUsedItem`](./battle%20globals.md#gLastUsedItem) to know what item the controller chose.

* When an NPC chooses to use an item, it sets flags in `gBattleStruct->AI_itemFlags` indicating the effect that the item will have, and it consumes the item. These flags are then acted upon by `HandleAction_UseItem` in order to actually apply the item's effects... indireectly, of course: it invokes a battle script based on what kind of effect the item will have, and the `useitemonopponent` script command actually applies the item's effects by way of a call to `PokemonUseItemEffects`.

[^item-use-callback]: An example of an item use callback is `ItemUseCB_Medicine`, defined in [`party_menu.c`](/src/party_menu.c). This callback is invoked as part of the Party Menu's tasks, after `gSpecialVar_ItemId` has already been set to the target item, and is responsible for checking whether a medicine item is usable, for actually executing the item's effect (by way of a call to `ExecuteTableBasedItemEffect`, which wraps `PokemonUseItemEffects`, which is what NPC items ultimately call down into as well), and for removing the item from the player's bag if it's single-use. `ItemUseCB_Medicine` makes no attempt whatsoever to communicate with the battle system, nor to leave any log of what actions it took (i.e. whether it consumed and applied the item).

### `CONTROLLER_HEALTHBARUPDATE`
The controller is being instructed to update `gActiveBattler`'s health bar; it should play the animation if appropriate, and wait for the animation to finish. (The Safari Zone controller is one example of it being inappropriate to play a health bar animation: the player isn't supposed to be using a Pokemon! Under the hood, they have a Pokemon deployed on the battlefield, but the Safari Zone controller refuses to display any of the associated graphics, so the player can't tell.)

The `gBattleBufferA[gActiveBattler]` buffer is laid out as follows:

| Offset | Type | Endianness | Description |
| -: | :-: | :-: | :- |
| 0 | `u8` | --- | Command ID: `CONTROLLER_HEALTHBARUPDATE`. |
| 1 | `u8` | --- | Padding (always zero). |
| 2 | `u16` | little-endian | Either the value that the battler's HP will soon be set to and to which the health bar should animate, or the constant `INSTANT_HP_BAR_DROP` indicating that the health bar should immediately snap to zero with no animation. |

Note that the Pokemon's current HP (as accessed via `GetMonData`) is out of date. This is useful: you know what value to animate from (its outdated current HP), what value to animate to (the value passed alongside this command), and you can get the max HP (via `GetMonData`) &mdash; all the information you need.

To actually animate the health bar, call `SetBattleBarStruct` (defined in [`battle_interface.h`](/include/battle_interface.h)). When snapping the health bar, pass zero instead of the real current HP, and (for the player) call `UpdateHpTextInHealthbox` (also in [`battle_interface.h`](/include/battle_interface.h)) to ensure the displayed HP number updates as well. Don't send a response until the health bar animation is complete.

Remember: you can access `Pokemon` data structures for use in `GetMonData` calls via `gEnemyParty[gBattlerPartyIndexes[gActiveBattler]]` and `gPlayerParty[gBattlerPartyIndexes[gActiveBattler]]`.

### `CONTROLLER_EXPUPDATE`
The controller is being instructed to give EXP to a Pokemon under its control, display the appropriate UI for it earning EXP, and then wait for any such UI to be dismissed.

The `gBattleBufferA[gActiveBattler]` buffer is laid out as follows:

| Offset | Type | Endianness | Description |
| -: | :-: | :-: | :- |
| 0 | `u8` | --- | Command ID: `CONTROLLER_EXPUPDATE`. |
| 1 | `u8` | --- | The Pokemon's slot within its trainer's party. |
| 2 | `u16` | little-endian | The amount of EXP that the Pokemon should be granted. |

If the party slot received is equal to `gBattlerPartyIndexes[gActiveBattler]`, then EXP is being granted to `gActiveBattler` (i.e. the Pokemon that is receiving EXP is out on the battlefield right now). Otherwise, the Pokemon that is receiving EXP is off the battlefield. This information can be used to decide what UI elements to display.

### `CONTROLLER_PLAYBGM`
An unused command: the local player controller handles it properly, but it's never emitted, and it may not always be *safe* to emit. Prefer `CONTROLLER_PLAYFANFAREORBGM` instead.

The controller is being asked to set the current background music. The music ID is the big-endian two-byte value at `&gBattleBufferA[gActiveBattler][1]`.

`BtlController_EmitPlayBGM`, the function which emits this command, builds the command data incorrectly. It's designed to send a song ID along with arbitrary additional data, but it accidentally treats the song ID as the size of the data rather than taking a `size` argument. This means that song IDs after `SE_RG_BAG_POCKET` (253) will write out of bounds.

### `CONTROLLER_CANTSWITCH`
A scrapped command: it's still emitted, but none of the controllers do anything in response to it.

The `openpartyscreen` battle script command is used to open the party menu. If it's called in response to a Pokemon fainting during a Double Battle, to prompt a trainer to send out a replacement, `openpartyscreen` can emit this controller command if the trainer has no available Pokemon they can switch in (e.g. because they only have the two Pokemon, or because all of their other Pokemon are fainted). This controller command gets emitted instead of the controller command that would lead to trainer having to choose a Pokemon.

Simply put: in any situation where `openpartyscreen` *would* prompt the player to choose a Pokemon to switch into a Double Battle, if they have no available Pokemon to switch in, then `CONTROLLER_CANTSWITCH` is emitted so that the local player controller can show a message, telling the player that they have no Pokemon to switch in. The actual message in question was scrapped in order to make battles flow better, so `CONTROLLER_CANTSWITCH` is basically scrapped.

### `CONTROLLER_PLAYFANFAREORBGM`
The controller is being asked to play a fanfare or set the background music.

The `gBattleBufferA[gActiveBattler]` buffer is laid out as follows:

| Offset | Type | Endianness | Description |
| -: | :-: | :-: | :- |
| 0 | `u8` | --- | Command ID: `CONTROLLER_PLAYFANFAREORBGM`. |
| 1 | `u16` | big-endian | A fanfare or music ID. |
| 3 | `bool8` | True if the ID is a song ID, to be passed to `PlayBGM`; false if it's a fanfare ID, to be passed to `PlayFanfare`. |

When this command contains a song ID, the controller should call `BattleStopLowHpSound` in addition to setting the background music.

This command is emitted by battle script commands &mdash; specifically, `fanfare` and the `VARIOUS_PLAY_TRAINER_DEFEATED_MUSIC` sub-command of `various`. Any controller can end up receiving the command, so they should all handle it consistently with one another. It's not clear to me why controllers are tasked with handling this, since it's not per-battler state.

### `CONTROLLER_INTROSLIDE`
At the start of a battle, the background art slides into view. This command kicks off that process: the controller should start the relevant animations and then finish handling the command *without* waiting for the animations to complete.

The `gBattleBufferA[gActiveBattler][1]` byte is a terrain ID &mdash; one of the `BATTLE_TERRAIN_` constants from [`constants/battle.h`](/include/constants/battle.h).

All controllers should handle this the same way: pass the terrain ID as an argument to `HandleIntroSlide` (from [`battle_anim.h`](include/battle_anim.h)); then `gIntroSlideFlags |= 1`; then finish handling the command. The battle engine emits this command toward the battler at [position](./battle%20concepts.md#battler-position) 0 on the field without caring what controller will end up receiving it (see `BattleIntroPrepareBackgroundSlide` in [`battle_main.c`](/src/battle_main.c)).

It's not clear to me why controllers are tasked with handling this, since it's not per-battler state. Maybe, at one point, Game Freak had intended for each battler to have their own background art? That would've made sense for situations like the player standing on grass and fishing up a Wild Pokemon. Getting that to work would probably require completely retooling how the background terrain art is implemented, though.

