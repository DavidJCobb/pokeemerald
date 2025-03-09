
# Battle controllers

A <dfn>battle controller</dfn> is a trainer participating in an ongoing battle. Battle controllers: are placed in charge of [battler positions](#battler-position) as appropriate; receive *commands*, such as "choose an action from the fight/PKMN/bag/run menu" or "play a Pokemon's fainting cry animation;" and perform actions in response to those commands. (Those actions can potentially include emitting a command in response.) Battle controllers are a key element in the abstractions that allow the same battle engine to handle real-time singleplayer battles, linked multiplayer battles, cutscene battles like Wally's capture of Ralts, and recorded battles. The battle engine can simply demand input from whoever or whatever is at the helm, and it doesn't have to care how those inputs are supplied &mdash; whether they're coming from the player, from an NPC, from over a multiplayer link, or what. The battle engine just ferries command data to where it needs to go and invokes the battle controller function pointer for each [battler](./battle%20concepts#battler-id) on the field.

Because battle controllers are tied to positions on the battlefield, the abstraction may not work the way you expect. For example, you might expect most player-facing GUI to be displayed by the local-player controller. However, some GUI elements are tied to battlefield positions, such as making a Pokemon's sprite flash when it takes damage, playing a Pokemon's cry when it faints, or even showing a trainer's party summary (the Poke Balls you see telling you how many Pokemon an opponent has); and so these would actually be performed by the NPC-opponent or link-opponent controllers.

Similarly, some battle controller commands may not immediately be intuitive. There are commands which instruct a battle controller to load and display a given battler's sprite, for example. Why are these handled by the controller, and not by the core battle engine? The Safari Zone is one reason. When facing off against Wild Pokemon in the Safari Zone, the player's half of the battlefield uses a special Safari Zone controller. Under the hood, the player's side of the field has all-zero Pokemon data[^safari-zone-null-player-mons] (which is not the same as nothing being there[^absent-flags]), but the Safari Zone controller refuses to load or display any of the associated graphics, in order to create the illusion that the player is facing off against the Wild Pokemon alone.

[^absent-flags]: The battle engine maintains a set of "absence flags" for each position on the battlefield.

[^safari-zone-null-player-mons]: You need to have a Pokemon on the field in order to be able to select an action to perform. However, if the Safari Zone had you send out your normal Pokemon and just hid them, then their abilities (e.g. Intimidate) would still activate, revealing that they're still there. It's safest to just have you unwittingly send out an invisible MISSINGNO. instead.

See, as far as the battle engine is concerned, you are *always* using Pokemon to battle some opposing Pokemon. If your battle controller chooses to hide that fact from you, while also only ever allowing you to select actions specific to the Safari Zone, that's not the battle system's business. Similarly, if the Wild Pokemon's battle controller checks the current battle type, sees that it's a Safari Zone battle, and chooses never to actually attack your (zeroed-out) Pokemon, then that's all fine and dandy as far as the battle system is concerned, and the system doesn't care why those decisions are being made.


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

<a name="emit"></a>The various `BtlController_Emit<Whatever>` functions in [`battle_controllers.h`](/include/battle_controllers.h)/[`.c`](/src/battle_controllers.c) use `sBattleBuffersTransferData` to build commands and then send them via `PrepareBufferDataTransfer`. When these commands are eventually received, they are then dispatched to the active battle controller, which must obey them. How are they dispatched?

Each [battler ID](./battle%20concepts.md#battler-id) has an associated callback function pointer for battle controllers. On every frame, `BattleMainCB1` invokes these functions for every battler, setting `gActiveBattler` to the battler in question.

Generally, when a controller is idle, its callback pointer is set to a function with a name like `<ThisControllerName>BufferRunCommand()`. This function checks whether the `gBattleControllerExecFlags` flag is set for `gActiveBattler`. If the flag is set, then `gBattleBufferA[gActiveBattler][0]` is treated as a [command](#commands) to run, and the appropriate function is called to handle that command. This function may be a [latent function](./battle%20concepts.md#latent), doing its work over multiple frames by overwriting the battler's callback pointer. When the command is finally handled, the controller will call `<ThisControllerName>BufferExecCompleted()`, which will set the battler's callback pointer back to `<ThisControllerName>BufferRunCommand`, before either transmitting data over the link cable (if the current battle is a Link Battle) or clearing the `gBattleControllerExecFlags` flag for `gActiveBattler`.

## Commands/messages
<a name="commands"></a><a name="messages"></a>

These are best thought of as messages, but are referred to in the codebase as "commands." Most messages are "inbound:" they are emitted by various parts of the battle engine, and the controller is expected to react to them in some way. Some messages are "outbound:" the controller itself emits them in response to messages it has received.

| Command | Direction | Description |
| :- | :-: | :- |
| [CONTROLLER_GETMONDATA](#CONTROLLER_GETMONDATA) | Inbound | Retrieve and send the data for a Pokemon. |
| [CONTROLLER_GETRAWMONDATA](#CONTROLLER_GETRAWMONDATA) | Inbound | Unused. Copy raw bytes from a Pokemon data structure (without decrypting them or anything) and respond to them. |
| CONTROLLER_SETMONDATA           | Inbound | Modify the data for a Pokemon. |
| CONTROLLER_SETRAWMONDATA        | Inbound | Unused. Overwrite a `BattlePokemon` with raw bytes. |
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
| [CONTROLLER_CHOOSEMOVE](#CONTROLLER_CHOOSEMOVE)           | Inbound | The controller responded to `CHOOSEACTION` by deciding to fight, so now it has to either pick a move to use and a target to use it on, or back out to the action menu and pick an action again. |
| [CONTROLLER_OPENBAG](#CONTROLLER_OPENBAG)                 | Inbound | The controller responded to `CHOOSEACTION` by deciding to use an item, so now it has to either pick an item, or back out to the action menu and pick an action again. |
| [CONTROLLER_CHOOSEPOKEMON](#CONTROLLER_CHOOSEPOKEMON)     | Inbound | The controller responded to `CHOOSEACTION` by deciding to switch Pokemon, so now it has to pick a Pokemon or cancel. If it's not currently possible to switch Pokemon, the controller must handle that condition here. |
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
| [CONTROLLER_LINKSTANDBYMSG](#CONTROLLER_LINKSTANDBYMSG) | Inbound | Sent to this battle controller in all battles (link and local), when the controller is likely to have made the last decision it can during the current turn. |
| CONTROLLER_RESETACTIONMOVESELECTION  | Inbound | |
| CONTROLLER_ENDLINKBATTLE             | Inbound | |
| CONTROLLER_TERMINATOR_NOP            | N/A | A no-op byte used to reset command buffers when they're not in use. Controllers deliberately do nothing in response to this command. |

### `CONTROLLER_GETMONDATA`
The controller is being directed to retrieve some of the persistent data for a Pokemon via a call to `GetMonData`.

The `gBattleBufferA[gActiveBattler]` buffer is laid out as follows:

| Offset | Type | Description |
| -: | :-: | :- |
| 0 | `u8` | Command ID: `CONTROLLER_GETMONDATA`. |
| 1 | `u8` | A request ID: one of the `REQUEST_` constants in [`battle_controllers.h`](/include/battle_controllers.h). |
| 2 | `u8` | Either zero, indicating that `gActiveBattler` is the Pokemon to access, or a bitmask indicating which party member(s) of `gActiveBattler`'s party to access. The vanilla implementation will hit a buffer overflow if retrieving data for more than two Pokemon simultaneously. |

Send your response by calling `BtlController_EmitDataTransfer(BUFFER_B, size, data)`. Multi-byte fields are encoded as little-endian. Fields use the smallest number of bytes possible (e.g. three bytes for the Pokemon's OT ID). Use the local-player controller implementation as a reference.[^getmondata-buffer]

[^getmondata-buffer] An oddity in how the local-player controller implementation was reverse-engineered: it allocates exactly 0x100 (256) bytes on the stack, which PRET documented as `sizeof(struct Pokemon) * 2 + 56`. (They seem to think Game Freak consciously made room for two and a half Pokemon structs. I think Game Freak just picked an "easy" buffer size that they'd never overflow under normal circumstances.) When implementing your own battle controllers, you need to allocate enough bytes on the stack to hold a `BattlePokemon` instance; I believe that's the largest struct that might ever be requested.

For completeness' sake, here's a list of places that can emit this message:

| Emitter | Source file | Reason |
| :- | :- | :- |
| `BattleIntroGetMonsData` | [`battle_main.c`](/src/battle_main.c) | Emits this message using `REQUEST_ALL_BATTLE` to retrieve each Pokemon's data on startup. The caller is latent, retrieving data for one Pokemon per frame. The controller's response is not read and handled immediately, but rather by `BattleIntroDrawTrainersOrMonsSprites` later in the battle intro process. |
| `PokemonUseItemEffects` | [`pokemon.c`](/src/pokemon.c) | When the player uses a healing item (`ITEM4_HEAL_HP`) that isn't a Revive on a Pokemon during a battle, `PokemonUseItemEffects` manually updates `gBattleMons[battlerId].hp` and then emits this message for the target battler. It seems like the response is never actually read, however: the player would use items during the local player controller's response to `CONTROLLER_OPENBAG`, so the battle engine would be waiting for a response to that message: it both isn't checking for, and has no way to recognize, a response to `CONTROLLER_GETMONDATA`. The battle engine knows a response to `CONTROLLER_OPENBAG` has been sent when the battle controller clears the relevant exec flag, so in practice, the battle controller's response to `CONTROLLER_OPENBAG` would overwrite the response to `CONTROLLER_GETMONDATA` in this situation. |

### `CONTROLLER_GETRAWMONDATA`
An unused and apparently broken command, unless it was meant for debugging, somehow.

The `gBattleBufferA[gActiveBattler]` buffer is laid out as follows:

| Offset | Type | Description |
| -: | :-: | :- |
| 0 | `u8` | Command ID: `CONTROLLER_GETRAWMONDATA`. |
| 1 | `u8` | Byte offset. |
| 2 | `u8` | Number of bytes to copy. |

The local player controller responds by copying bytes out of a `Pokemon` instance (`gPlayerParty[gBattlerPartyIndices[gActiveBattler]]` and blidnly overwriting bytes in the middle of a stack-allocated `BattlePokemon` instance. It then sends the overwritten portion of that instance as a response using `BtlController_EmitDataTransfer(BUFFER_B, gBattleBufferA[gActiveBattler][2], dst)`.

On the one hand, the source and destination used for the copy operation have incompatible struct layouts. On the other hand, it shouldn't actually matter, because the controller should only be sending the portion of the data that it actually copied; in essence, the local `BattlePokemon` instance is treated like a raw buffer of `sizeof(BattlePokemon)` bytes. But this is all just terribly janky, especially since `Pokemon` instances are packed and encrypted so the data wouldn't even be legible to the recipient.

### `CONTROLLER_SETMONDATA`
The controller is being directed to modify the persistent data for a Pokemon via a call to `SetMonData`. This message is emitted to handle all changes to a Pokemon, such as changes to its HP as a result of taking damage, and changes to its PP as a result of effects like Pressure.

The `gBattleBufferA[gActiveBattler]` buffer is laid out as follows:

| Offset | Type | Description |
| -: | :-: | :- |
| 0 | `u8` | Command ID: `CONTROLLER_SETMONDATA`. |
| 1 | `u8` | A request ID: one of the `REQUEST_` constants in [`battle_controllers.h`](/include/battle_controllers.h). |
| 2 | `u8` | Either zero, indicating that `gActiveBattler` is the Pokemon to modify, or a bitmask indicating which party member(s) of `gActiveBattler`'s party to modify. |
| 3 | varies | The data to write, as a raw buffer. |

For most request types, you can just call `SetMonData` and pass `&gBattleBufferA[gActiveBattler][3]`. However, some types require special handling:

| Response | Data type | Details |
| :- | :- | :- |
| `REQUEST_ALL_BATTLE` | `BattlePokemon` | Since `BattlePokemon` and `Pokemon` are laid out differently, and the former contains lots of transient (battle-scoped) state, you'll want to use `SetMonData` for each persistent field individually. |
| `REQUEST_MOVES_PP_BATTLE` | `MovePpInfo` | Generally invoked as the result of things like Pressure increasing a Pokemon's PP usage. |
| `REQUEST_PP_DATA_BATTLE` | `u8[5]` | Five bytes. In order: the PP for each of a Pokemon's moves, followed by a single byte representing the PP bonuses (i.e. PP Up and PP Max increases) for all four moves. |
| `REQUEST_ALL_IVS_BATTLE` | `u8[6]` | Six bytes: the IVs for each stat in this order: HP, Attack, Defense, Speed, Special Attack, and Special Defense. |

For completeness' sake, here's a list of places that can emit this message:

| Emitter | Source file | Reason |
| :- | :- | :- |
| `BattlePalace_TryEscapeStatus` | [`battle_util2.c`](/src/battle_util2.c) | In the Battle Palace, your Pokemon act autonomously without your orders. They may escape status effects on their own. |
| `PressurePPLose` | [`battle_util.c`](/src/battle_util.c) | Checks if an attacker is using a move on a target that has the Pressure ability. If so, the attacker loses 1 more PP than usual. If the move is persistent &mdash; if it wasn't acquired via Mimic, Transform, or a similar effect &mdash; then this message is emitted so that the PP loss applies outside of battle. |
| `PressurePPLoseOnUsingImprison` | [`battle_util.c`](/src/battle_util.c) | Similar to `PressurePPLose`, but specific to the move Imprison being used. |
| `PressurePPLoseOnUsingPerishSong` | [`battle_util.c`](/src/battle_util.c) | Similar to `PressurePPLose`, but specific to the move Perish Song being used. |
| `DoBattlerEndTurnEffects` | [`battle_util.c`](/src/battle_util.c) | The function as a whole loops over all attackers (treating each as `gActiveBattler`) and processes all end-of-turn effects. The `ENDTURN_UPROAR` case handler checks if an attacker is asleep and lacks the Soundproof ability and if so, emits this message to wake them up at the end of a turn. |
| `DoBattlerEndTurnEffects` | [`battle_util.c`](/src/battle_util.c) | The function as a whole loops over all attackers (treating each as `gActiveBattler`) and processes all end-of-turn effects. The `ENDTURN_YAWN` case handler emits this message to apply the Sleep status to a Pokemon. |
| `AtkCanceller_UnableToUseMove` | [`battle_util.c`](/src/battle_util.c) | This function handles effects that may "cancel" a Pokemon's usage of a move (e.g. a Pokemon being unable to attack because it is frozen): it handles both cancelling the move use, and checking whether the thing that would cancel it has ended (e.g. thawing a Pokemon when it gets hit by certain moves). When appropriate, the function emits this message to clear a Pokemon's status effect. |

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
| `B_ACTION_USE_MOVE` | Use a move (the "Fight" option in the player-facing top-level menu). After you give this response, you should expect to receive a `CONTROLLER_CHOOSEMOVE` command. This response isn't binding: you can change your mind upon receiving `CONTROLLER_CHOOSEMOVE` and ask to choose a different action. |
| `B_ACTION_USE_ITEM` | Use an item (the "Bag" option in the player-facing top-level menu). After you give this response, you should expect to receive a `CONTROLLER_OPENBAG` command. This response isn't binding: you can change your mind upon receiving `CONTROLLER_OPENBAG` and ask to choose a different action. |
| `B_ACTION_SWITCH` | <p>Switch out this battler for a different party member. After you give this response, you should expect to receive a `CONTROLLER_CHOOSEPOKEMON` command. This response isn't binding: you can change your mind upon receiving `CONTROLLER_CHOOSEPOKEMON` and ask to choose a different action.</p><p>You can choose this action even if something is preventing `gActiveBattler` from switching out. This is intentional: this action is also used to let the player inspect their own party via the party menu. The `CONTROLLER_CHOOSEPOKEMON` message that you'll receive down the line will tell you whether you're allowed to switch out, and if not, why.</p> |
| `B_ACTION_RUN` | Attempt to flee. This action is used by both the player and by Wild Pokemon. |
| `B_ACTION_SAFARI_WATCH_CAREFULLY` | This is the "do nothing" action that Wild Pokemon perform in the Safari Zone. |
| `B_ACTION_SAFARI_BALL` | Throw a Safari Ball at a Wild Pokemon in the Safari Zone. |
| `B_ACTION_SAFARI_POKEBLOCK` | Throw a PokeBlock at a Wild Pokemon in the Safari Zone. After you give this response, you should expect to receive a `CONTROLLER_OPENBAG` command. |
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

#### Special cases for NPCs

You can call `BtlController_EmitTwoReturnValues(BUFFER_B, action, 0)` for any of the following actions (but no others), to commit to those actions (instead of using a move) without having to cancel out of move selection and wait for another `CONTROLLER_CHOOSEACTION` message. NPC AI uses this for the "watch carefully" and "run" actions, and Wally's battle controller uses the functionality as well:

* `B_ACTION_RUN`
* `B_ACTION_SAFARI_WATCH_CAREFULLY`
* `B_ACTION_SAFARI_BALL`
* `B_ACTION_SAFARI_POKEBLOCK`
* `B_ACTION_SAFARI_GO_NEAR`
* `B_ACTION_SAFARI_RUN`
* `B_ACTION_WALLY_THROW`

You can also call `BtlController_EmitTwoReturnValues(BUFFER_B, 15, battler)` to make the specified `battler` switch out. This assumes that a switch-in Pokemon has already been written to `gBattleStruct->monToSwitchIntoId[gActiveBattler]`. This functionality is used by NPC AIs, but the function that handles it also checks for Link Multi Battles (strangely, the link battle controllrs don't use it).

### `CONTROLLER_OPENBAG`
The controller responded to `CHOOSEACTION` by deciding to use an item, so now it has to pick an item to use and an owned or allied battler to use it on.

Choose an item (or PokeBlock) to use by calling `BtlController_EmitOneReturnValue(BUFFER_B, itemID)`. To back out of the item selection menu (as the player themselves can), use zero as the item ID.

Note that the battle engine doesn't appear to be fully responsible for *actually using* the item:

* When the local player chooses to use an item, the item use callback[^item-use-callback], if one exists, is invoked as part of the item selection process, before the item ID is sent to the battle system: the effects of the item are applied before the battle system ever gets a chance to know what's happened, and the battle system isn't specifically told what those effects were. This, it seems, is why items aren't usable in Link Battles: all item effects are designed to run locally, with no provisions made to let them synch.

  That said, there are some items that are specifically designed to integrate into the battle system exclusively, such as Poke Balls, Poke Dolls, Fluffy Tails, and so on. These are handled by `HandleAction_UseItem`, defined in [`battle_util.c`](/src/battle_util.c). That function relies on [`gLastUsedItem`](./battle%20globals.md#gLastUsedItem) to know what item the controller chose.

* When an NPC chooses to use an item, it sets flags in `gBattleStruct->AI_itemFlags` indicating the effect that the item will have, and it consumes the item. These flags are then acted upon by `HandleAction_UseItem` in order to actually apply the item's effects... indirectly, of course: it invokes a battle script based on what kind of effect the item will have, and the `useitemonopponent` script command actually applies the item's effects by way of a call to `PokemonUseItemEffects`.

[^item-use-callback]: An example of an item use callback is `ItemUseCB_Medicine`, defined in [`party_menu.c`](/src/party_menu.c). This callback is invoked as part of the Party Menu's tasks, after `gSpecialVar_ItemId` has already been set to the target item, and is responsible for checking whether a medicine item is usable, for actually executing the item's effect (by way of a call to `ExecuteTableBasedItemEffect`, which wraps `PokemonUseItemEffects`, which is what NPC items ultimately call down into as well), and for removing the item from the player's bag if it's single-use. `ItemUseCB_Medicine` makes no attempt whatsoever to communicate with the battle system, nor to leave any log of what actions it took (i.e. whether it consumed and applied the item).

### `CONTROLLER_CHOOSEPOKEMON`
The controller responded to `CHOOSEACTION` by deciding to switch Pokemon, so now it has to pick a Pokemon to send out. Alternatively, the controller can cancel and return to choosing an action.

The `gBattleBufferA[gActiveBattler]` buffer is laid out as follows:

| Offset | Type | Description |
| -: | :-: | :- |
| 0 | `u8` | Command ID: `CONTROLLER_CHOOSEPOKEMON`. |
| 1 | `u8:4` | A party action constant (`PARTY_ACTION_`) defined in [`constants/party_menu.h`](/include/constants/party_menu.h). This includes sentinel values indicating when the battler is being prevented from switching out. |
| 1 | `u8:4` | The [battler ID](./battle%20concepts.md#battler-id) of a Pokemon preventing `gActiveBattler` from switching out. This is an informative value used for printing error messages; prefer the party action constant to know when switchout is being prevented, not this value. |
| 2 | `u8` | The party slot that the battler is in. The party menu relies on this being copied to `gBattleStruct->prevSelectedPartySlot`, so it can block you from switching a Pokemon out for itself. |
| 3 | `u8` | If `gActiveBattler` is being prevented from switching out, then this indicates which ability (`ABILITY_` constant in [`constants/abilities.h`](/include/constants/abilities.h)) is preventing switchout. You can write it to `gLastUsedAbility` for printing in battle strings. This is an informative value used for printing error messages; prefer the party action constant to know when switchout is being prevented, not this value. |
| 4 | `u8[6]` | A mapping between the order of the trainer's party on the overworld, and the current order within battle. |

You'll want to store the party mapping order somewhere, so you can pass it as an argument to `BtlController_EmitChosenMonReturnValue` when choosing to switch Pokemon. The local player controller copies it to `gBattlePartyCurrentOrder`, and the party menu relies on it being copied there.

The local player can choose a Pokemon to send out by calling `BtlController_EmitChosenMonReturnValue(BUFFER_B, index, gBattlePartyCurrentOrder)`. Alternatively, call `BtlController_EmitChosenMonReturnValue(BUFFER_B, PARTY_SIZE, NULL)` to change your mind and ask to perform a different action instead.

The third argument, when giving a response, is a mapping to

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

All controllers should[^introslide-no-data] handle this the same way: pass the terrain ID as an argument to `HandleIntroSlide` (from [`battle_anim.h`](include/battle_anim.h)); then `gIntroSlideFlags |= 1`; then finish handling the command. The battle engine emits this command toward the battler at [position](./battle%20concepts.md#battler-position) 0 on the field without caring what controller will end up receiving it (see `BattleIntroPrepareBackgroundSlide` in [`battle_main.c`](/src/battle_main.c)).

[^introslide-no-data]: If you want to experiment and try to implement something new, then go nuts, but be aware: your controller *must not* respond with any data. The [battle intro](./battle%20execution%20flow.md) uses `CONTROLLER_GETMONDATA` to load data for the four Pokemon initially on the battlefield, but doesn't bother to read that data until after `CONTROLLER_INTROSLIDE` has been handled. Responding to this message will overwrite your prior response to the `CONTROLLER_GETMONDATA` message, causing the battle engine to initialize `gBattleMons` with corrupt data.

It's not clear to me why controllers are tasked with handling this, since it's not per-battler state. Maybe, at one point, Game Freak had intended for each battler to have their own background art? That would've made sense for situations like the player standing on grass and fishing up a Wild Pokemon. Getting that to work would probably require completely retooling how the background terrain art is implemented, though.

### `CONTROLLER_LINKSTANDBYMSG`
The controller may have made the last decision it can make during this turn, and is now being told to get ready to wait. (For example, in a Double Battle, the local player controller may receive this message after its right-flank Pokemon has committed to an action.) In a Link Battle, this is when you would display the "Link standby..." message, but do be aware that despite its name, this message will be received during local battles as well.

The `gBattleBufferA[gActiveBattler]` buffer is laid out as follows:

| Offset | Type | Endianness | Description |
| -: | :-: | :-: | :- |
| 0 | `u8` | --- | Command ID: `CONTROLLER_LINKSTANDBYMSG`. |
| 1 | `u8` | --- | One of the `LINK_STANDBY_` constants from [`battle_controllers.h`](/include/battle_controllers.h). |
| 2 | varies | --- | Data to be passed to the battle-recording system. |

Your controller should call `RecordedBattle_RecordAllBattlerData(&gBattleBufferA[gActiveBattler][2])`, and then check the link standby mode and handle it accordingly:

| Mode | Description |
| :- | :- |
| `LINK_STANDBY_MSG_ONLY` | Check if the current battle is a Link Battle, and only if so, display the "Link standby..." battle string. |
| `LINK_STANDBY_STOP_BOUNCE_ONLY` | Perform the same actions as you would for `CONTROLLER_ENDBOUNCE`. |
| `LINK_STANDBY_MSG_STOP_BOUNCE` | Perform both of the above two actions. |

