
## `gActionSelectionCursor`

This array's indices are [battler IDs](./battle concepts.md#battler-id).


## `gActiveBattler`

A [battler ID](./battle concepts.md#battler-id).


## `gBattleBufferA` and `gBattleBufferB`
<a name="gbattlebuffera"></a><a name="gbattlebufferb"></a>

Two data buffers used to transfer commands that are being emitted to [battle controllers](./battle concepts.md#battle-controller). Each buffer is divided into one slot per [battler ID](./battle concepts.md#battle-id), with each slot able to hold 512 bytes of data.

Known uses for these slots include:

* Storing `BattleMsgData` instances, which are read by `BufferStringBattle` in [`battle_message.c`](/src/battle_message.c) and used to supply parameters to various predefined strings.
* Storing `ChooseMoveStruct` instances, which are read by `ChooseMoveAndTargetInBattlePalace` in [`battle_gfx_sfx_util.c`](/src/battle_gfx_sfx_util.c) and `SetPpNumbersPaletteInMoveSelection` in [`battle_message.c`](/src/battle_message.c).
* Storing data being emitted to a battle controller by the various `BtlController_Emit<Whatever>` functions in [`battle_controllers.h`](/include/battle_controllers.h)/[`.c`](/src/battle_controllers.c).

For link battles, data may be copied from `gLinkBattleRecvBuffer` into either of the two battle buffers by `Task_HandleCopyReceivedLinkBuffersData` in [`battle_controllers.c`](/src/battle_controllers.c).


## `gBattleCommunication`

Defined in `battle.h`, this is a raw buffer of size `BATTLE_COMMUNICATION_ENTRIES_COUNT`[^BATTLE_COMMUNICATION_ENTRIES_COUNT]. The defined indices are as follows:

<table>
   <thead>
      <tr>
         <th>Index</th>
         <th>Index constant[^gBattleCommunication_Indices]</th>
         <th>Description</th>
      </tr>
   </thead>
   <tbody>
      <tr>
         <td>0</td>
         <td><code>MULTIUSE_STATE</code></td>
         <td>
            <p>Various uses:</p>
            <ul>
               <li>
                  <p>The <code>CB2_HandleStartBattle</code> function is a frame handler designed as 
                  a state machine, in order to stagger battle-related setup tasks across multiple 
                  frames and prevent the game from becoming unresponsive. Even in non-link battles, 
                  <code>gBattleCommunication[MULTIUSE_STATE]</code> is used as the state variable.</p>
               </li>
            </ul>
         </td>
      </tr>
      <tr>
         <td>1</td>
         <td><code>CURSOR_POSITION</code></td>
         <td>...</td>
      </tr>
      <tr>
         <td>1</td>
         <td><code>TASK_ID</code></td>
         <td>...</td>
      </tr>
      <tr>
         <td>1</td>
         <td><code>SPRITES_INIT_STATE1</code></td>
         <td>...</td>
      </tr>
      <tr>
         <td>2</td>
         <td><code>SPRITES_INIT_STATE2</code></td>
         <td>...</td>
      </tr>
      <tr>
         <td>3</td>
         <td><code>MOVE_EFFECT_BYTE</code></td>
         <td>...</td>
      </tr>
      <tr>
         <td>4</td>
         <td><code>ACTIONS_CONFIRMED_COUNT</code></td>
         <td>...</td>
      </tr>
      <tr>
         <td>5</td>
         <td><code>MULTISTRING_CHOOSER</code></td>
         <td>...</td>
      </tr>
      <tr>
         <td>6</td>
         <td><code>MISS_TYPE</code></td>
         <td>...</td>
      </tr>
      <tr>
         <td>7</td>
         <td><code>MSG_DISPLAY</code></td>
         <td>...</td>
      </tr>
   </tbody>
</table>


[^BATTLE_COMMUNICATION_ENTRIES_COUNT]: `BATTLE_COMMUNICATION_ENTRIES_COUNT` == 8. It's defined in `include/constants/battle_script_commands.h`.

[^gBattleCommunication_Indices]: The names for these indices are defined in `include/constants/battle_script_commands.h`.


## `gBattleControllerExecFlags`

State flags for the [battle controller](./battle concepts.md#battle-controller) system.

| Bit indices | Total count | Meaning |
| - | -: | :- |
| *y* | 4 | The current battle is not a link battle, and [battler ID](./battle concepts.md#battler-id) *y* has been marked for controller exec[^MarkBattlerForControllerExec] |
| 4*x* + *y* | 16  | Player *x* has received[^MarkBattlerReceivedLinkData] link data for [battler ID](./battle concepts.md#battler-id) *y* |
| 28 + *y* | 4 | A link battle is ongoing, [battler ID](./battle concepts.md#battler-id) *y* has been marked for controller exec[^MarkBattlerForControllerExec], and link data for that battler has not yet been synchronized[^MarkBattlerReceivedLinkData] |

[^MarkBattlerForControllerExec]: See `MarkBattlerForControllerExec` in [`battle_util.c`](/src/battle_util.c).

[^MarkBattlerReceivedLinkData]: See `MarkBattlerReceivedLinkData` in [`battle_util.c`](/src/battle_util.c).


## `gBattleMons`

An array of `BattlePokemon` (type defined in [`pokemon.h`](/include/pokemon.h)), one per [position on the battlefield](./battle concepts.md#battler-position). [Battler IDs](./battle concepts.md#battler-id) are indices into this array.


## `gBattlerControllerFuncs`

An array where the indices are [battler IDs](./battle concepts.md#battler-id) and the values are function pointers to be invoked (to run a [battle controller](./battle concepts.md#battle-controller) for that battler) by the battle system.


## `gBattlerPositions`

An array of battlers' positions on the field; indices are [battler IDs](./battle concepts.md#battler-id).


## `gDisableStructs`

An array of `DisableStruct` instances; indices are [battler IDs](./battle concepts.md#battler-id).

Members:

| Type | Name | Description |
| :- | :- | :- |
| `u32` | `transformedMonPersonality` | If this battler has used Transform, then this value is the personality value of the target they transformed into. |
| `u16` | `disabledMove` |
| `u16` | `encoredMove` | The move that this battler is currently being forced to use as a result of having been hit with Encore. |
| `u8` | `protectUses` | The number of consecutive times this battler has used moves with the Protect or Endure effects. This value is used to control the likelihood of these moves failing[^Cmd_setprotectlike], by being treated as an index into an array of success rates which, concerningly, is neither bounds-checked nor terminated with a 0%-success entry. |
| `u8` | `stockpileCounter` |
| `u8` | `substituteHP` | The health of this Pokemon's active Substitute, if any. If this field is zero by the end of a turn[^TurnValuesCleanUp], then the battler's `STATUS2_SUBSTITUTE` flag will be cleared. This field is preserved when a Pokemon switches in via Baton Pass. |
| `u8:4` | `disableTimer` |
| `u8:4` | `disableTimerStartValue` |
| `u8` | `encoredMovePos` | The position of the move that this battler is currently being forced to use as a result of having been hit with Encore, i.e. if they're being forced to use the *n*th move then this is *n*. This is used to detect and handle the case of this battler forgetting the encored move (by checking if the move at this position in their moveset is still equal to `encoredMove`). |
| `u8` | `filler_D` | Unused field. |
| `u8:4` | `encoreTimer` |
| `u8:4` | `encoreTimerStartValue` |
| `u8:4` | `perishSongTimer` | This field is preserved when a Pokemon switches in via Baton Pass. |
| `u8:4` | `perishSongTimerStartValue` | This field is preserved when a Pokemon switches in via Baton Pass. |
| `u8` | `furyCutterCounter` |
| `u8:4` | `rolloutTimer` |
| `u8:4` | `rolloutTimerStartValue` |
| `u8:4` | `chargeTimer` |
| `u8:4` | `chargeTimerStartValue` |
| `u8:4` | `tauntTimer` |
| `u8:4` | `tauntTimer2` |
| `u8` | `battlerPreventingEscape` | The [ID](./battle concepts.md#battler-id) of an opposing battler, set in conjunction with `STATUS2_ESCAPE_PREVENTION` by the handler for `MOVE_EFFECT_PREVENT_ESCAPE`. These values together are used to ensure that the battler to which this struct belongs is unable to switch out or escape battle unless and until the battler to which this field refers is taken off the battlefield. This field is preserved when a Pokemon switches in via Baton Pass. |
| `u8` | `battlerWithSureHit` | The [ID](./battle concepts.md#battler-id) of an opposing battler, set in conjunction with `STATUS3_ALWAYS_HITS` by the battle script command `Cmd_setalwayshitflag`. These values together allow the referred-to attacker to always hit the battler to which this struct belongs. This field is preserved when a Pokemon switches in via Baton Pass. |
| `u8` | `isFirstTurn` | <p>This field's value is non-zero during the first potentially actionable turn that a Pokemon spends on the battlefield. The reason it's a `u8` and not a `bool8` is because it gets set to 2 when a Pokemon switches in, presumably because the function[^TurnValuesCleanUp] that decrements non-zero values at the end of each turn hasn't run yet.</p><p>Battle AI script commands can query this value[^Cmd_is_first_turn_for], and it also influences hardcoded AI actions, preventing NPCs[^ShouldUseItem] from using X-Items or Guard Spec on a Pokemon they've only just switched in.</p> |
| `u8` | `filler_17` | Unused field. |
| `u8:1` | `truantCounter` | A cycling counter used to track when this battler's actions should be blocked by the Truant ability, should it have said ability. |
| `u8:1` | `truantSwitchInHack` | This is the only field on this struct that is never cleared when a Pokemon switches in. Various battle script commands manage the flag as a hack to manage the `truantCounter` when a Pokemon switches in after a lost judgment in the Battle Arena. |
| `u8:2` | `filler_18_2` | Padding bits. |
| `u8:4` | `mimickedMoves` | A bitfield identifying which of the battler's four moves were replaced via Mimic[^Cmd_mimicattackcopy] or a similar effect. |
| `u8` | `rechargeTimer` | If this is non-zero, it's decremented at the end of each turn[^TurnValuesCleanUp], and should it then reach zero, the Pokemon will have `STATUS2_RECHARGE` removed. |

[^Cmd_is_first_turn_for]: See `Cmd_is_first_turn_for` in `battle_ai_script_commands.c`.

[^Cmd_mimicattackcopy]: See `Cmd_mimicattackcopy` in [`battle_script_commands.c`](/src/battle_script_commands.c).

[^Cmd_setprotectlike]: See `Cmd_setprotectlike` in [`battle_script_commands.c`](/src/battle_script_commands.c).

[^ShouldUseItem]: See `ShouldUseItem` in `battle_ai_switch_items.c`.

[^TurnValuesCleanUp]: See `TurnValuesCleanUp` in [`battle_main.c`](/src/battle_main.c).


## `gLastUsedItem`

Presumably, the last item used by any participant in the battle.

The `ItemBattleEffects`[^ItemBattleEffects] function is responsible for checking for and executing hold items' effects for any given battler. When invoked, the function immediately sets `gLastUsedItem` to the battler's held item, even if no effect ends up being executed.

[^ItemBattleEffects]: `ItemBattleEffects` is defined in `battle_util.c`, and is responsible for checking for and executing hold items' effects for any given battler. The function is invoked with an item effect event (e.g. "on switch in"), a [battler ID](./battle concepts.md#battler-id), and a boolean indicating whether we're in a part of the current turn where moves are being processed (basically). Item effect events use the `ITEMEFFECT_` enum defined in `battle_util.h`.


## `gLinkBattleSendBuffer`

A [pointer to a] heap-allocated buffer used to send data over a multiplayer link on behalf of the battle controllers. The data written inside uses the following layout, assuming little-endian:

| Type | Member | Info |
| - | - | - |
| `u8` | `bufferID` | The local buffer IDs are `BUFFER_A` (0) or `BUFFER_B` (1) for the local `gBattleBufferA` and `gBattleBufferB`, and 2 refers to the link buffer.
| `u8` | [`gActiveBattler`](#gactivebattler) | When `BUFFER_A` is received, players other than the link master will overwrite `gActiveBattler` with the value received here, and all players will call `MarkBattlerReceivedLinkData(battlerId)` with the battler ID received here. ALternatively, when buffer 2 is specified, the recipient will treat the first byte of `data` as a player index, and will clear the `gBattleControllerExecFlags` flag for the received `gActiveBattler` and for that player (in essence marking themselves as awaiting that data from that player). |
| `u8` | `gBattlerAttacker` | When `BUFFER_A` is received, players other than the link master will overwrite `gBattlerAttacker` with the value received here. |
| `u8` | `gBattlerTarget` | When `BUFFER_A` is received, players other than the link master will overwrite `gBattlerTarget` with the value received here. |
| `u16` | `size` | Little-endian. Size of the variable-length data at the end of the structure. |
| `u8` | `gAbsentBattlerFlags` | When `BUFFER_A` is received, players other than the link master will overwrite `gAbsentBattlerFlags` with the value received here. |
| `u8` | `gEffectBattler` | When `BUFFER_A` is received, players other than the link master will overwrite `gEffectBattler` with the value received here. |
| ... | `data` | Any other data that needs to be sent. This will be written to `gBattleBufferA` or `gBattleBufferB` if either is specified by the message's `bufferID`. |

Data is written to this buffer by `PrepareBufferDataTransferLink` in [`battle_controllers.c`](/src/battle_controllers.c), and later sent by `Task_HandleSendLinkBuffersData` (same file). The tasks for sending and receiving link data are always running in the background and seemingly wait until they have data to send. The tasks rely on task-local data fields that no one has given names, and I can't quite make heads or tails of them.

### Unverified: task behavior

The "send" task has the following behavior, using task-data fields. Task-data field `data[11]` is the state enum for the task handler:

* State 0: queue a wait for 100 frames and go to state 1.
* State 1: wait for `data[10]` many frames and then advance to state 2.
* State 2: advance to state 3 if we're on wireless, *or* if all link cable players register as connected and it's safe for us to advance.
* State 3:
  * Wait until `data[15] != data[14]`.
  * Wait for `data[13]` many frames.
  * If `data[15] > data[14] && data[15] == data[12]`, then set `data[12]` and `data[15]` to 0.
  * Send a block of data from offset `data[15]` into the `gLinkBattleSendBuffer` buffer.
  * Advance to state 4.
* State 4:
  * Wait until `IsLinkTaskFinished()`.
  * Reset `data[13]` to 1.
  * Advance `data[15]` by the size of the data we've sent.
  * Return to state 3.

* `data[10]` is the number of frames to wait while idling.
* `data[11]` is the state variable for the task handler.
* `data[12]` is the padded amount of data to send?
* `data[13]` is the number of frames we should delay before sending any data.
* `data[14]` is the amount of data to send?
* `data[15]` is an offset into `gLinkBattleSendBuffer`; we begin sending data from this offset over the link.


## `gMoveSelectionCursor`

This array's indices are [battler IDs](./battle concepts.md#battler-id).


## `gMoveToLearn`

A `u16` identifying the move that a Pokemon is currently trying to learn as a result of leveling up. This global is defined in [`battle.h`](/include/battle.h) and used both inside and outside of the battle engine:

* `MonTryLearningNewMove` in [`pokemon.c`](/src/pokemon.c) is responsible for checking whether a Pokemon has a move they can learn at their current level and if so, writing that move into `gMoveToLearn` and returning it as a result.
* The Party Menu is heavily involved in the level-up and move-learning processes.
* The daycare and the code for Pokemon evolution both touch this variable for obvious reasons.
* Several [battle script commands](/src/battle_script_commands.c) touch the variable:
  * `Cmd_buffermovetolearn`
  * `Cmd_yesnoboxlearnmove`
    * State 2 brings the player to the Moves page of a Pokemon's summary screen, passing `gMoveToLearn` as the move to learn.
    * State 4 checks whether the player chose an existing move their Pokemon is allowed to overwrite (i.e. not an HM move), and if so, modifies their Pokemon (both persistent data and temporary battle data) to write in the new move.


## `gMultiPartnerParty`

In Multi Battles[^dfn-multi-battle], this data structure holds the party data for the player's ally trainer. It appears to be a mirror to `sMultiPartnerPartyBuffer`, which is heap-allocated on demand.

The data structure is initialized by function `SetMultiPartnerMenuParty(u8 offset)`[^SetMultiPartnerMenuParty]. This function has two callers: `CB2_PreInitMultiBattle`, for Link Multi Battles, and `CB2_PreInitIngamePlayerPartnerBattle`, for singleplayer Multi Battles wherein the player has an NPC ally.


[^dfn-multi-battle]: A Multi Battle is any 4v4 battle, including both PvP 4v4 (wherein the player teams up with another player, against two opposing players) and PvE 4v4 (wherein the player teams up with an NPC ally against two NPC enemies).

[^SetMultiPartnerMenuParty]: Defined in `battle_main.c`. This function initializes `gMultiPartnerParty` by copying half of the player's party (`gPlayerParty`), with its argument intended to indicate which half. (The argument is actually measured in Pokemon, not in party halves, and the function doesn't bounds-check as it reads from `gPlayerParty`.) Afterwards, it uses `memcpy` to overwrite `sMultiPartnerPartyBuffer` with `gMultiPartnerParty`.


## `gPaydayMoney`

The amount (`u16`) of money that this battler's winner will earn as a result of the move Pay Day. The value is increased by the handler for `MOVE_EFFECT_PAYDAY`.