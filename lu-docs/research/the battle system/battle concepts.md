
## Battle concepts

### Battle controller

A <dfn>battle controller</dfn> is a trainer participating in an ongoing battle. Battle controllers: are placed in charge of [battler positions](#battler-position) as appropriate; receive *commands*, such as "choose an action from the fight/item/bag/run menu" or "play a Pokemon's fainting cry animation;" and perform actions in response to those commands. Battle controllers are a key element in the abstractions that allow the same battle engine to handle real-time singleplayer battles, linked multiplayer battles, cutscene battles like Wally's capture of Ralts, and recorded battles. The battle engine doesn't have to know or care *how* it's being given commands &mdash; whether they're coming from the player, from an NPC, from over a multiplayer link, or what. The battle engine just ferries command data to where it needs to go and invokes the battle controller function pointer for each [battler](#battler-id) on the field.

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

The various `BtlController_Emit<Whatever>` functions in [`battle_controllers.h`](/include/battle_controllers.h)/[`.c`](/src/battle_controllers.c) use `sBattleBuffersTransferData` to build commands and then send them via `PrepareBufferDataTransfer`. When these commands are eventually received, they are then passed to the active battle controller, which must obey them.


#### Battle controller functions

Battle controllers function similarly to the task system, wherein each [battler ID](#battler-id) has an associated function pointer belonging to a battle controller (stored in `gBattlerControllerFuncs`), and this pointer is invoked on every frame by `BattleMainCB1`. This essentially means that battle controllers' responses to commands are asynchronous, allowing controllers to respond to commands based on events that may occur at an indefinite point in the future (e.g. player menu interactions, multiplayer link data transfers).

Generally, the entry point for a controller is a function with a name like `<Controller>BufferRunCommand`, which checks whether the `gBattleControllerExecFlags` flag is set for `gActiveBattler`. If the flag is set, then byte `gBattleBufferA[gActiveBattler][0]` is treated as a [command](#battle-controller-commands) to run, and the appropriate function is called to handle that command. Once the command is sufficiently handled (or if it's invalid), `<Controller>BufferExecCompleted()` is called, which sets the callback for `gActiveBattler` back to the entry point function, before either transmitting data over the link cable (if the current battle is a Link Battle) or clearing the `gBattleControllerExecFlags` flag for `gActiveBattler`.


#### Battle controller commands

Battle controllers need to be able to handle the following commands:

| Command | Description |
| :- | :- |
| CONTROLLER_GETMONDATA | |
| CONTROLLER_GETRAWMONDATA | |
| CONTROLLER_SETMONDATA | |
| CONTROLLER_SETRAWMONDATA | |
| CONTROLLER_LOADMONSPRITE | |
| CONTROLLER_SWITCHINANIM | |
| CONTROLLER_RETURNMONTOBALL | |
| CONTROLLER_DRAWTRAINERPIC | |
| CONTROLLER_TRAINERSLIDE | Slide into view the sprite for the trainer to whom `gActiveBattler` belongs. |
| CONTROLLER_TRAINERSLIDEBACK | Slide out of view the sprite for the trainer to whom `gActiveBattler` belongs. |
| CONTROLLER_FAINTANIMATION | Play a fainting animation for `gActiveBattler`. If this is the local player controller, then now would be a good time to update the low HP sound as well. |
| CONTROLLER_PALETTEFADE | |
| CONTROLLER_SUCCESSBALLTHROWANIM | Play the animation for throwing a Poke Ball from `gActiveBattler`'s position on the battlefield, with ball throw case ID `BALL_3_SHAKES_SUCCESS`. For the local player controller, the ball is assumed to have been thrown at `B_POSITION_OPPONENT_LEFT`. |
| CONTROLLER_BALLTHROWANIM | Play the animation for throwing a Poke Ball from `gActiveBattler`'s position on the battlefield. The ball throw case ID (indicating the number of shakes) is in `gBattleBufferA[gActiveBattler][1]` and is one of the values in the `BALL_` enum in [`battle_controllers.h`](/include/battle_controllers.h). For the local player controller, the ball is assumed to have been thrown at `B_POSITION_OPPONENT_LEFT`. |
| CONTROLLER_PAUSE | Pause briefly? `gBattleBufferA[gActiveBattler][1]` is a delay value, but the only controller that actually heeds it is the local player controller; and that controller doesn't delay for *n* many frames, but rather just does `while (timer != 0) --timer;`. |
| CONTROLLER_MOVEANIMATION | |
| CONTROLLER_PRINTSTRING | Display text to the player. `*(u16*)&gBattleBufferA[gActiveBattler][2]` is a battle string ID. Controllers for the player and their ally should generally display the string and then wait until the text printer is finished. |
| CONTROLLER_PRINTSTRINGPLAYERONLY | Similar to `CONTROLLER_PRINTSTRING`, except the string should only be displayed for the player: if `gActiveBattler` is not on `B_SIDE_PLAYER`, then do nothing. |
| CONTROLLER_CHOOSEACTION | The controller should decide whether to use an attack, use an item, switch out, or try to run &mdash; the top-level battle menu. This isn't the time to decide *which* attack, or item, or teammate. To make your choice, call `BtlController_EmitTwoReturnValues(BUFFER_B, choice, 0)`. where `choice` is the appropriate `B_ACTION_` constant from [`battle.h`](/include/battle.h). |
| CONTROLLER_YESNOBOX | The controller should respond to a yes/no dialog box. To make your choice, call `BtlController_EmitTwoReturnValues(BUFFER_B, choice, 0)`, where `choice` is `0xD` for yes or `0xE` for no. |
| [CONTROLLER_CHOOSEMOVE](#CONTROLLER_CHOOSEMOVE) | The controller responded to `CHOOSEACTION` by deciding to fight, so now it has to either pick a move to use and a target to use it on, or back out to the action menu and pick again. |
| CONTROLLER_OPENBAG | |
| CONTROLLER_CHOOSEPOKEMON | |
| CONTROLLER_23 | |
| CONTROLLER_HEALTHBARUPDATE | |
| CONTROLLER_EXPUPDATE | |
| CONTROLLER_STATUSICONUPDATE | |
| CONTROLLER_STATUSANIMATION | |
| CONTROLLER_STATUSXOR | |
| CONTROLLER_DATATRANSFER | |
| CONTROLLER_DMA3TRANSFER | |
| CONTROLLER_PLAYBGM | The controller is being asked to set the current background music. The music ID is the big-endian two-byte value at `&gBattleBufferA[gActiveBattler][1]`. |
| CONTROLLER_32 | |
| CONTROLLER_TWORETURNVALUES | |
| CONTROLLER_CHOSENMONRETURNVALUE | |
| CONTROLLER_ONERETURNVALUE | |
| CONTROLLER_ONERETURNVALUE_DUPLICATE | |
| CONTROLLER_CLEARUNKVAR | |
| CONTROLLER_SETUNKVAR | |
| CONTROLLER_CLEARUNKFLAG | |
| CONTROLLER_TOGGLEUNKFLAG | |
| CONTROLLER_HITANIMATION | The controller should play the animation for `gActiveBattler` being hit (e.g. flashing its sprite and calling `DoHitAnimHealthboxEffect(gActiveBattler)`, if the battler is not invisible). The controller should wait for the animation to complete before proceeding further. |
| CONTROLLER_CANTSWITCH | |
| CONTROLLER_PLAYSE | The controller is being asked to play a sound effect. The sound ID is the big-endian two-byte value at `&gBattleBufferA[gActiveBattler][1]`. The controller need not wait for the sound to complete. |
| CONTROLLER_PLAYFANFAREORBGM | The controller is being asked to play a fanfare or set the background music. The fanfare or music ID is the big-endian two-byte value at `&gBattleBufferA[gActiveBattler][1]`, while the single-byte bool at `gBattleBufferA[gActiveBattler][3]` indicates whether it's as fanfare (false) or music (true). |
| CONTROLLER_FAINTINGCRY | The controller should play the fainting cry sound effect for `gActiveBattler`. It need not wait for the sound to complete. |
| CONTROLLER_INTROSLIDE | |
| CONTROLLER_INTROTRAINERBALLTHROW | |
| CONTROLLER_DRAWPARTYSTATUSSUMMARY | |
| CONTROLLER_HIDEPARTYSTATUSSUMMARY | |
| CONTROLLER_ENDBOUNCE | |
| CONTROLLER_SPRITEINVISIBILITY | The controller should update the invisibility state for `gActiveBattler`'s sprite, setting it to `gBattleBufferA[gActiveBattler][1]`. The controller should make the appropriate checks to verify that there's even a sprite there to update, first. |
| CONTROLLER_BATTLEANIMATION | |
| CONTROLLER_LINKSTANDBYMSG | |
| CONTROLLER_RESETACTIONMOVESELECTION | |
| CONTROLLER_ENDLINKBATTLE | |
| CONTROLLER_TERMINATOR_NOP | A no-op byte used to reset command buffers when they're not in use. Controllers deliberately do nothing in response to this command. |

##### `CONTROLLER_CHOOSEMOVE`
The controller responded to `CHOOSEACTION` by deciding to fight, so now it has to pick a move to use and a target to use it on.

The `gBattleBufferA[gActiveBattler]` buffer is laid out as follows:

| Offset | Type | Description |
| -: | - | - |
| 0 | `u8` | Command ID: `CONTROLLER_CHOOSEMOVE`. |
| 1 | `bool8` | Indicates that this battle is a Double Battle. |
| 2 | `bool8` | In single battles, indicates that the controller should not be allowed to choose the target of a `MOVE_TARGET_USER_OR_SELECTED` move (as they would normally be able to). Whether it indicates anything else, I don't know. |
| 3 | `u8` | unknown/padding |
| 4 | `ChooseMoveStruct` | The moves that the controller has available to choose from, along with their current and max PP, the species of the Pokemon for which they're choosing a move, and the types of that Pokemon. |

If it's a controller for the local player, you can allow them to reorder moves here; just be sure to update the `ChooseMoveStruct` associated with this command, the move data for `gBattleMons[gActiveBattler]`, and the persistent (non-battle) Pokemon data (i.e. `gPlayerParty[gBattlerPartyIndexes[gActiveBattler]]`). Doesn't look like there's any need to synchronize these sorts of changes over a multiplayer link, at least not immediately.

To choose a move to use, call `BtlController_EmitTwoReturnValues(BUFFER_B, 10, choice | (target << 8))`, where `choice` is the index of the move to use, and `target` is the [battler position](#battler-position) to attack.

Alternatively, to back out of the move selection menu (as the player themselves can), call `BtlController_EmitTwoReturnValues(BUFFER_B, 10, 0xFFFF)`.


### Battler ID

A `u8` identifying a battler currently on the field. Battler IDs are always in the range \[0, `MAX_BATTLERS_COUNT`) i.e. \[0, 4) and are indices into the `gBattleMons` array. Battler IDs are functionally equivalent to [battler positions](#battler-position).

Generally speaking, `0xFF` is used to indicate "none" wherever "no battler" is an appropriate value for a battler ID.


### Battler position

The battlefield is divided into four total positions for combatants:

| Constant | Value | Value (binary) |
| - | -: |
| `B_POSITION_PLAYER_LEFT` | 0 | `0b00` |
| `B_POSITION_OPPONENT_LEFT` | 1 | `0b01` |
| `B_POSITION_PLAYER_RIGHT` | 2 | `0b10` |
| `B_POSITION_OPPONENT_RIGHT` | 3 | `0b11` |

The least-significant bit of a position represents a <dfn>side</dfn> of the battlefield. `B_SIDE_PLAYER` (0) is the local player's side of the battlefield, and `B_SIDE_OPPONENT` (1) is the opposite side of the field.

The next bit of the position represents the left (`B_FLANK_LEFT` == 0) or right (`B_FLANK_RIGHT` == 1) flank.

Note that "left" and "right" are described relative to each side. The opponent is facing you, so their left-side battler is drawn on the righthand side of the screen, and vice versa.


### Status

There are three different status enums, all defined in [`constants/battle.h`](/include/constants/battle.h).

#### `STATUS1`: Persistent status conditions
<a name="status1"></a>

Pokemon that attain these status conditions will continue to possess them in some form after switching out, and after the battle ends. During battle, they're stored in `BattlePokemon::status1`.

| Constant | Bitcount | Bits | Description |
| :- | -: | - | :- |
| `STATUS1_NONE`          | 0 | `0b000000000000` | Indicates the absence of any status-1. |
| `STATUS1_SLEEP`         | 3 | `0b000000000111` | Indicates that the Pokemon is asleep, and acts as a counter denoting how many turns they will remain asleep for. |
| `STATUS1_POISON`        | 1 | `0b000000001000` | Indicates that the Pokemon is poisoned. |
| `STATUS1_BURN`          | 1 | `0b000000010000` | Indicates that the Pokemon is burned. |
| `STATUS1_FREEZE`        | 1 | `0b000000100000` | Indicates that the Pokemon is frozen. |
| `STATUS1_PARALYSIS`     | 1 | `0b000001000000` | Indicates that the Pokemon is paralyzed. |
| `STATUS1_TOXIC_POISON`  | 1 | `0b000010000000` | Indicates that the Pokemon is badly poisoned: their poison damage will worsen over time. |
| `STATUS1_TOXIC_COUNTER` | 1 | `0b111100000000` | A counter which increases each turn for Pokemon that are badly poisoned: this is used to track by how much their poison damage has worsened. |
| `STATUS1_PSN_ANY`       | 2 | `0b000010001000` | Indicates that the Pokemon has any kind of poison status (ordinarily or badly poisoned). |

#### `STATUS2`: Volatile status conditions
<a name="status2"></a>

Pokemon that attain these status conditions will lose them after switching out, and after the battle ends. They're stored in `BattlePokemon::status2`.

| Constant | Bitcount | Bits | Description |
| :- | -: | -: | :- |
| `STATUS2_CONFUSION`         | 3 | `0b00000000000000000000000000000111` | Indicates that the Pokemon is confused, and acts as a counter denoting how many turns they will remain confused for before snapping out of it. |
| `STATUS2_FLINCHED`          | 1 | `0b00000000000000000000000000001000` | Indicates that the Pokemon is flinching. |
| `STATUS2_UPROAR`            | 3 | `0b00000000000000000000000001110000` | Indicates that the Pokemon is making an uproar, and acts as a counter denoting how many turns they will continue making an uproar for. |
| `STATUS2_UNUSED`            | 1 | `0b00000000000000000000000010000000` | Unused. |
| `STATUS2_BIDE`              | 2 | `0b00000000000000000000001100000000` |
| `STATUS2_LOCK_CONFUSE`      | 2 | `0b00000000000000000000110000000000` |
| `STATUS2_MULTIPLETURNS`     | 1 | `0b00000000000000000001000000000000` |
| `STATUS2_WRAPPED`           | 3 | `0b00000000000000001110000000000000` |
| `STATUS2_INFATUATION`       | 4 | `0b00000000000011110000000000000000` | Each bit indicates a battler that the Pokemon is infatuated with. |
| `STATUS2_FOCUS_ENERGY`      | 1 | `0b00000000000100000000000000000000` |
| `STATUS2_TRANSFORMED`       | 1 | `0b00000000001000000000000000000000` |
| `STATUS2_RECHARGE`          | 1 | `0b00000000010000000000000000000000` |
| `STATUS2_RAGE`              | 1 | `0b00000000100000000000000000000000` |
| `STATUS2_SUBSTITUTE`        | 1 | `0b00000001000000000000000000000000` |
| `STATUS2_DESTINY_BOND`      | 1 | `0b00000010000000000000000000000000` |
| `STATUS2_ESCAPE_PREVENTION` | 1 | `0b00000100000000000000000000000000` | Indicates that this Pokemon is being prevented from leaving the battlefield by another battler. Refer to [`gDisableStructs[this_battler].battlerPreventingEscape`](./battle globals.md#gdisablestructs) for that battler's identity. |
| `STATUS2_NIGHTMARE`         | 1 | `0b00001000000000000000000000000000` |
| `STATUS2_CURSED`            | 1 | `0b00010000000000000000000000000000` |
| `STATUS2_FORESIGHT`         | 1 | `0b00100000000000000000000000000000` |
| `STATUS2_DEFENSE_CURL`      | 1 | `0b01000000000000000000000000000000` |
| `STATUS2_TORMENT`           | 1 | `0b10000000000000000000000000000000` |

#### `STATUS3`: Miscellaneous
<a name="status3"></a>

These status values are stored in `gStatuses3`.

Given that they're stored differently than the other status data, and they refer to conditions that are even more ephemeral than those in `STATUS2`, it may perhaps be inappropriate to regard these values as "statuses," as if they, `STATUS1`, and `STATUS2` are in any way "of the same." However, I'm as unable to come up with a specific category for them as PRET (per their comments) were.

| Constant | Bitcount | Bits | Description |
| :- | -: | -: | :- |
| `STATUS3_LEECHSEED_BATTLER` | 2 | `0b000000000000000000011` | Two bits identifying the battler whose HP will be absorbed by this battler. |
| `STATUS3_LEECHSEED`         | 1 | `0b000000000000000000100` |
| `STATUS3_ALWAYS_HITS`       | 2 | `0b000000000000000011000` | A countdown to an imminent attack which will be guaranteed to hit this battler. Refer to [`gDisableStructs[this_battler].battlerWithSureHit`](./battle globals.md#gdisablestructs) for the attacker's identity. |
| `STATUS3_PERISH_SONG`       | 1 | `0b000000000000000100000` |
| `STATUS3_ON_AIR`            | 1 | `0b000000000000001000000` | Indicates that this Pokemon is high in the air (e.g. from using Fly) and cannot be hit by conventional attacks. |
| `STATUS3_UNDERGROUND`       | 1 | `0b000000000000010000000` | Indicates that this Pokemon is underground (e.g. from using Dig) and cannot be hit by conventional attacks. |
| `STATUS3_MINIMIZED`         | 1 | `0b000000000000100000000` | Indicates that this Pokemon has used Minimize at least once. |
| `STATUS3_CHARGED_UP`        | 1 | `0b000000000001000000000` |
| `STATUS3_ROOTED`            | 1 | `0b000000000010000000000` |
| `STATUS3_YAWN`              | 2 | `0b000000001100000000000` | Indicates that the Pokemon has been hit with Yawn, and acts as a counter denoting how many turns remain until they fall asleep. |
| `STATUS3_IMPRISONED_OTHERS` | 1 | `0b000000010000000000000` |
| `STATUS3_GRUDGE`            | 1 | `0b000000100000000000000` |
| `STATUS3_CANT_SCORE_A_CRIT` | 1 | `0b000001000000000000000` |
| `STATUS3_MUDSPORT`          | 1 | `0b000010000000000000000` |
| `STATUS3_WATERSPORT`        | 1 | `0b000100000000000000000` |
| `STATUS3_UNDERWATER`        | 1 | `0b001000000000000000000` | Indicates that this Pokemon is concealed underwater (e.g. from using Dive) and cannot be hit by conventional attacks. |
| `STATUS3_INTIMIDATE_POKES`  | 1 | `0b010000000000000000000` | Indicates that this Pokemon is currently intimidating other Pokemon. The status is managed by `AbilityBattleEffects` in `battle_util.c`, being set by the handler for `ABILITY_INTIMIDATE` and cleared by the handlers for `ABILITYEFFECT_INTIMIDATE1` and `ABILITYEFFECT_INTIMIDATE2`. |
| `STATUS3_SEMI_INVULNERABLE` | 1 | `0b001000000000011000000` | Convenience constant for testing whether any of the "semi-invulnerable" statuses are active. |


## Link concepts

### Link Master

In Halo games (prior to the franchise adopting dedicated servers for its multiplayer), this would be called the "connection host." Their game state is considered canonical. The link master is chosen by `FindLinkBattleMaster` in [`battle_main.c`](/src/battle_main.c), which selects as follows (in order of descending priority):

* Player 1, if they're using the minimum link version (0x0100)
* Player 1, if the linked players are all using the same link version
* The first-seen player who's using a link version greater than 0x0300 and who has a multiplayer ID lower than that of the local player
* The lowest-index player using link version 0x0300 who isn't the local player
* The local player

During a link battle, you can tell if the local player is the link master by checking `(gBattleTypeFlags & BATTLE_TYPE_IS_MASTER) != 0`, though this check also passes for all non-link battles.


## General concepts

### Work amortized over multiple frames
<a name="amortized"></a>

Several hardcoded tasks and script commands distribute their computation over multiple frames, instead of running start-to-finish in one shot. Sometimes, this is done because a function or process must wait on events occurring elsewhere, such as UI interactions or link connectivity. Other times, though, code that could easily just run start-to-finish is instead built to distribute its computation across frames, presumably to avoid potential performance issues &mdash; frame rate lag, audio stutters, and the like.

The general pattern is this: the amortized function is built as a switch-case statement, switching on a "state" variable of some sort. The variable is typically a counter, being incremented at the end of each case, though this won't be the case when branching execution is needed. The amortized function, then, is called once per frame until its work completes. Any variables that are needed across multiple stages of computation (including the "state" variable itself) can't be local (because they'll be lost at the end of each frame) and must be stored elsewhere.

As for *how* a function gets itself to be called once per frame, there are two common cases:

* Some processes are implemented as frame callback handlers (the functions whose names are prefixed with `CB2`).
* For battle scripts, the script instruction pointer doesn't advance automatically; battle script commands must advance it manually. If they don't advance the instruction pointer, then they'll just run again on the next frame.

It's useful to have a piece of jargon that instantly refers to this one specific pattern and not anything else in the codebase, so I've personally chosen to use the word <dfn>amortize</dfn> to refer to this it: "to reduce a debt or cost by paying small regular amounts," defined in contrast to paying a large lump sum. In this case, Game Freak often appears to be "amortizing" the performance costs of long computations and processes, preferring "small regular amounts" of computation across multiple frames.

Examples include but are most certainly not limited to...

* There are several functions that set up the initial state for battles, depending on the battle type (Link Battle, Multi Battle, etc.). These amortize their work over multiple frames and generally use `gBattleCommunication[MULTIUSE_STATE]` as their state variable.
* The `getexp` battle script command amortizes its work over multiple frames. It uses `gBattleScripting.getexpState` as its state variable, and uses `gBattleStruct->expValue` and `gBattleStruct->expGetterBattlerId` to track data that gets used across multiple stages of computation.
* The `trygivecaughtmonnick` battle script command amortizes its work over multiple frames, because it has to wait on GUI events and interactions. It uses `gBattleCommunication[MULTIUSE_STATE]` as its state variable, and relies on `gBattleStruct->caughtMonNick` to hold the player's chosen nickname for a caught Wild Pokemon.


