
[link-battle-controller]: ./battle%20controllers.md
[link-battler-id]: ./battle%20concepts.md#battler-id
[link-latent-functions]: ./battle%20concepts.md#latent

# Battle execution flow

The core frame handler for battles is `BattleMainCB1` in [`battle_main.c`](/src/battle_main.c). This handler calls the `gBattleMainFunc` function pointer, which is likely to be a [latent function][link-latent-functions], before then calling the [battle controller][link-battle-controller] function pointers for each active battler (setting `gActiveBattler` to the [battler ID][link-battler-id] it's invoking the pointer for).

This means, then, that up to five latent functions may be running simultaneously:

* Current battle-engine latent function
* Latent function for battler 0's battle controller
* Latent function for battler 1's battle controller
* Latent function for battler 2's battle controller
* Latent function for battler 3's battle controller

The `gBattleControllerExecFlags` bitmask indicates when a battle controller has work to do. The controller callbacks are always invoked, but will generally return immediately if the flag for `gActiveBattler` isn't set. When the battle engine wants to send messages to the controllers, it'll generally [emit](./battle%20controllers.md#emit) the message to `BUFFER_A` and call `MarkBattlerForControllerExec(gActiveBattler)`. When the battle engine wants to wait until all outstanding messages to controllers have been processed, it'll wait (`return`) until `gBattleControllerExecFlags` is all zeroes.

During local battles, controllers clear the controller-exec flag for a battler when they've finished processing inbound messages for it. During link battles, however, they do something different: they send their player ID over the link[^sending-player-id], and then overwrite the last received message's identifier with `CONTROLLER_TERMINATOR_NOP`. (Overwriting the command with a nop means that until the controller-exec flag is cleared, the controller will just keep sending their player ID over the link as a completion signal, on every frame.)

[^sending-player-id]: Via a call to `PrepareBufferDataTransferLink(2, 4, &playerId)`. This is the only time data is ever sent over buffer ID 2 rather than `BUFFER_A` (0, for messages to controllers) or `BUFFER_B` (1, for outbound responses from controllers).

## `gBattleControllerExecFlags`

The bits in `gBattleControllerExecFlags` have the following meaning:

| Set | Bit indices | Total count | Summary | Meaning |
| :-: | :-: | -: | :- | :- |
| A | <var>y</var> | 4 | Local-exec flag | **The current battle is not a link battle, and [battler ID][link-battler-id] <var>y</var> has been marked for controller exec.** During local battles, the flag is set by `MarkBattlerForControllerExec` and manually cleared by the battler's controller when the controller has finished processing an inbound message. |
| ABCD | 4<var>x</var> + <var>y</var> | 16 | Message-available flag | <p>**We have received inbound message data for [battler ID][link-battler-id] <var>y</var>'s battle controller, and this data is yet to be processed on player <var>x</var>'s machine.** The flags for battler <var>y</var> (all players) are set[^MarkBattlerReceivedLinkData] when we receive data destined for `BUFFER_A`, at the time the data is copied to <code>gBattleBufferA[<var>y</var>]</code>. The flag for player <var>x</var> battler <var>y</var> cleared when the battle controller running for that battler on that machine finishes processing the inbound message (it will at that point send a message for buffer ID 2 containing the multiplayer ID for player <var>x</var>, and receipt of this message clears the flag).</p><p>Note that the message-available flags for player 0 overlap the local-exec flags; this means that battle controllers will run locally whenever inbound message data is received. This isn't a problem: battle controllers for link participants are designed as no-ops. When Alice and Bob battle, the link-opponent controller on Alice's machine does nothing, while the local-player controller on Bob's machine sends his commands via `BUFFER_B`, to be acted on by the battle engine both locally and on Alice's machine.</p> |
| WXYZ | 28 + <var>y</var> | 4 | Link-exec flag | A link battle is ongoing and [battler ID][link-battler-id] <var>y</var> has been marked for controller exec[^MarkBattlerForControllerExec], but inbound message data for that battler has not yet been synchronized[^MarkBattlerReceivedLinkData]. During link battles, this flag is set by `MarkBattlerForControllerExec` (instead of the local-exec flag), and cleared[^MarkBattlerReceivedLinkData] when we receive an inbound message for this battler. |

The `MarkBattlerForControllerExec` function is used to indicate that a battler's controller should respond to an inbound message, though during link battles, it won't immediately take effect until the inbound message data is actually received. During link battles, `Task_HandleCopyReceivedLinkBuffersData` is responsible for receiving battle controller communications &mdash; specifically, inbound message data (`BUFFER_A`), outbound message data (`BUFFER_B`), and the completion signals (buffer ID 2) for each player's battle controllers. It updates the exec-flags in response to this received data.

[^MarkBattlerForControllerExec]: See `MarkBattlerForControllerExec`, which sets either a local-exec flag or a link-exec flag depending on whether the battle is a link battle. Even the controllers for the local player will use the link-exec flags in link battles.

[^MarkBattlerReceivedLinkData]: See `MarkBattlerReceivedLinkData`, which sets the message-available flag and clears the link-exec flag for a given battler ID.

## Battle engine latent functions

The `gBattleMainFunc` pointer may be set to any of the following functions:

| Function | File | Explanation |
| :- | :- | :- |
| `BattleIntroGetMonsData` | [`battle_main.c`](/src/battle_main.c) | Written by `BattleBeginIntro`. |
| `BattleIntroPrepareBackgroundSlide` | [`battle_main.c`](/src/battle_main.c) | Written by `BattleIntroGetMonsData`. |
| `BattleIntroDrawTrainersOrMonsSprites` | [`battle_main.c`](/src/battle_main.c) | Written by `BattleIntroPrepareBackgroundSlide`. |

### `BeginBattleIntro`

* Set callback to latent function `BattleIntroGetMonsData`. That function sends `CONTROLLER_GETMONDATA` messages to all battle controllers, and advances to `BattleIntroPrepareBackgroundSlide` after all battle controllers have responded. The responses will be present in `gBattleBufferB`, but they won't be read immediately; a later function will read them.
* Latent function `BattleIntroPrepareBackgroundSlide` sends `CONTROLLER_INTROSLIDE` to the battle controller for battler 0, and advances to `BattleIntroDrawTrainersOrMonsSprites` after the battle controller has responded.
* Latent function `BattleIntroDrawTrainersOrMonsSprites` runs code for each battler on the field...
  * For Safari Zone battles, it zeroes the `gBattleMons` structs for all Pokemon on `B_SIDE_PLAYER`.
  * For all other battle types, `gBattleMons` is initialized as appropriate, extracting from `gBattleBufferB` the data that the battle controllers previously sent in response to `CONTROLLER_GETMONDATA`.
  * For the battler at `B_POSITION_PLAYER_LEFT`, we emit a `CONTROLLER_DRAWTRAINERPIC` message.
  * For the battler at `B_POSITION_OPPONENT_LEFT`...
    * In a trainer battle, we emit a `CONTROLLER_DRAWTRAINERPIC` message.
    * In a non-trainer battle, we emit a `CONTROLLER_LOADMONSPRITE` message.
  * In a Multi Battle, we emit `CONTROLLER_DRAWTRAINERPIC` messages for `B_POSITION_PLAYER_RIGHT` and `B_POSITION_OPPONENT_RIGHT`.
  * For battle type `BATTLE_TYPE_TWO_OPPONENTS`, we emit a `CONTROLLER_DRAWTRAINERPIC` message for `B_POSITION_OPPONENT_RIGHT`.
  * Wait for all battle controllers to respond, and then advance to `BattleIntroDrawPartySummaryScreens`.
* Latent function `BattleIntroDrawPartySummaryScreens` checks if this is a trainer battle and if so, it emits `CONTROLLER_DRAWPARTYSTATUSSUMMARY` messages to the battle controllers for `B_POSITION_OPPONENT_LEFT` and `B_POSITION_PLAYER_LEFT`. Either way, it waits for all battle controllers to respond (if it gave them anything to respond to) before advancing to either `BattleIntroPrintTrainerWantsToBattle` or `BattleIntroPrintWildMonAttacked`.
  * Trainer branch:
    * Latent function `BattleIntroPrintTrainerWantsToBattle` (redundantly?) waits until all extant battle controllers are quiet, and then emits a `CONTROLLER_PRINTSTRING` message to `B_POSITION_OPPONENT_LEFT` and advances immediately to `BattleIntroPrintOpponentSendsOut`.
    * Latent function `BattleIntroPrintOpponentSendsOut` emits a `CONTROLLER_PRINTSTRING` message to one of the controllers &mdash; usually the one for `B_POSITION_OPPONENT_LEFT`, but for recorded link battles where the recording player isn't the [Link Master](./battle%20concepts.md#link-master), for `B_POSITION_PLAYER_LEFT` instead. We then advance immediately to `BattleIntroOpponent1SendsOutMonAnimation`.
    * Latent function `BattleIntroOpponent1SendsOutMonAnimation` (redundantly?) waits until all extant battle controllers are quiet, and then plays the send-out animation for the lefthand-side opponent, by emitting a `CONTROLLER_INTROTRAINERBALLTHROW` message to the appropriate battle controller (same logic as previous). It then advances immediately &mdash; either to `BattleIntroOpponent2SendsOutMonAnimation`, for Multi Battles or 1v2 Battles; or to `BattleIntroRecordMonsToDex`.
      * Latent function `BattleIntroOpponent2SendsOutMonAnimation` emits `CONTROLLER_INTROTRAINERBALLTHROW` for the righthand-side opponent (picking the controller similarly to the above) and then advances immediately to `BattleIntroRecordMonsToDex`.
    * Latelt function `BattleIntroRecordMonsToDex` waits until all controllers are quiet, and then loops over each battler. If the current battle isn't recorded, in the Battle Frontier, in Trainer Hill, a link battle, or an eReader battle, then it sets the "seen" flag for the battlers on `B_SIDE_OPPONENT`. Once its work is done, it advances to `BattleIntroPrintPlayerSendsOut`.
  * Wild branch:
    * Latent function `BattleIntroPrintWildMonAttacked` waits until all extant battle controllers are quiet, and then emits a `CONTROLLER_PRINTSTRING` message to `B_POSITION_OPPONENT_LEFT` and advances immediately to `BattleIntroPrintPlayerSendsOut`.
* Latent function `BattleIntroPrintPlayerSendsOut` emits a `CONTROLLER_PRINTSTRING` message to one of the controllers &mdash; usually the one for `B_POSITION_PLAYER_LEFT`, but for recorded link battles where the recording player isn't the [Link Master](./battle%20concepts.md#link-master), for `B_POSITION_OPPONENT_LEFT` instead. We then advance immediately to `BattleIntroPlayer1SendsOutMonAnimation`.
* Latent function `BattleIntroPlayer1SendsOutMonAnimation` (redundantly?) waits until all extant battle controllers are quiet, and then emits `CONTROLLER_INTROTRAINERBALLTHROW` for the player and advances immediately &mdash; to either `BattleIntroPlayer2SendsOutMonAnimation`, in Multi Battles, or `TryDoEventsBeforeFirstTurn`.
  * This function also clears [`gBattleStruct->switchInAbilitiesCounter`](./battle%20types%20-%20BattleStruct.md#switchinabilitiescounter), [`gBattleStruct->switchInItemsCounter`](./battle%20types%20-%20BattleStruct.md#switchinitemscounter), and [`gBattleStruct->overworldWeatherDone`](./battle%20types%20-%20BattleStruct.md#overworldWeatherDone).
  * Latent function `BattleIntroPlayer2SendsOutMonAnimation` emits `CONTROLLER_INTROTRAINERBALLTHROW` for the player's ally, clears the same three above variables, and advances immediately to `TryDoEventsBeforeFirstTurn`.
* Latent function `TryDoEventsBeforeFirstTurn` is quite advanced...
  * Wait for all battle controllers to respond to any outstanding messages.
  * Handle switch-in abilities, latently.
    * If the loop hasn't started yet, then compute the order in which abilities should be executed (fastest to slowest Pokemon).
    * Handle [`gBattleStruct->overworldWeatherDone`](./battle%20types%20-%20BattleStruct.md#overworldWeatherDone).
    * Loop over the battlers and execute latent function `AbilityBattleEffects` to process switch-in abilities. The loop itself is latent: if `AbilityBattleEffects` returns a non-zero value, then an ability executes, and we return early so the loop can continue on the next frame.
  * Handle Intimidate (`ABILITYEFFECT_INTIMIDATE1`) and Trace (`ABILITYEFFECT_TRACE`).
  * Handle switch-in hold items, latently.
    * Loop over the battlers and execute latent function `ItemBattleEffects` to process switch-in item effects. The loop itself is latent: if `ItemBattleEffects` returns a non-zero value, then an item executes, and we return early so the loop can continue on the next frame.
  * For each battler, clear `gBattleStruct->monToSwitchIntoId[i]`, `gChosenActionByBattler[i]`, and `gChosenMoveByBattler[i]`.
  * `TurnValuesCleanUp(FALSE)`
  * `SpecialStatusesClear()`
  * Copy `gAbsentBattlerFlags` to `gBattleStruct->absentBattlerFlags`.
  * Clear the battle text window.
  * Advance to latent function [`HandleTurnActionSelectionState`](#HandleTurnActionSelectionState) (but we're not done yet; there's a bit more cleanup to do).
  * Zero out `gBattleCommunication`.
  * Clear [`STATUS2_FLINCHED`](./battle%20concepts.md#status2) from all battlers.
  * Clear the latent state for [end-of-turn effects](./battle%20types%20-%20BattleStruct.md#turnEffectsTracker-and-turnEffectsBattlerId), Wish and Perisoh Song state, move-end scripting state, and `faintedActionsState` and `turnCountersTracker` on `gBattleStruct`.
  * Compute `gRandomTurnNumber`.
  * If this battle is `BATTLE_TYPE_ARENA`, then clera all Pokemon cries, and execute the `BattleScript_ArenaTurnBeginning` battle script. With how scripts execute (see `BattleScriptExecute`), this will delay the execution of [`HandleTurnActionSelectionState`](#HandleTurnActionSelectionState) until the battle script is complete.

### Turn actions

#### `HandleTurnActionSelectionState`
Each battler must select an action. Once all battlers have selected an action, we advance to `SetActionsAndBattlersTurnOrder`. We track the number of battlers who have selected an action in `gBattleCommunication[ACTIONS_CONFIRMED_COUNT]`, comparing it to `gBattlersCount`.

The overall action states are:

<table>
   <thead>
      <tr>
         <th>State</th>
         <th>Description>
      </tr>
   </thead>
   <tbody>
      <tr>
         <td><code>STATE_TURN_START_RECORD</code></td>
         <td>Start of a turn in a recorded battle. Handle recorded battle bookkeeping and then advance to <code>STATE_BEFORE_ACTION_CHOSEN</code>.</td>
      </tr>
      <tr>
         <td><code>STATE_BEFORE_ACTION_CHOSEN</code></td>
         <td>
            <p>The battler has yet to choose an action. This function will want until it's time for them to choose an action, and then either auto-select an action or command the battler's controller to choose.</p>
            <p>The moment a left-flank battler enters this action state, it's time for them to select an action. In Double Battles, the right-flank battler always choose an action after the left-flank battler has made their choice. In Multi Battles, the battlers on the left and right flanks may choose concurrently.</p>
            <p>If an action is auto-selected, then the battler advances immediately to <code>STATE_WAIT_ACTION_CONFIRMED_STANDBY</code> or <code>STATE_WAIT_ACTION_CONFIRMED</code>. Alternatively, if the battle controller is instructed to select an action, the battler advances to <code>STATE_WAIT_ACTION_CHOSEN</code>.</p>
         </td>
      </tr>
      <tr>
         <td><code>STATE_WAIT_ACTION_CHOSEN</code></td>
         <td>
            <p>The battler's battle controller has been asked to select an action, and we are now waiting for a response. We wait until all of the <code>gBattleControllerExecFlags</code> for this battler are cleared, and until the link-exec flags for all battlers are cleared. Then, we assume that a response is available in <code>gBattleBufferB</code>.</p>
            <p>Once that response is received, we check the action it contains. For actions with a sub-case, we emit the appropriate battle controller commands (e.g. if the player chose to use a move, we emit <code>CONTORLLER_CHOOSEMOVE</code>), and for actions without, we just carry the action out. By default, actions will advance us to <code>STATE_WAIT_ACTION_CONFIRMED_STANDBY</code> (not directly, but rather by incrementing the state variable).</p>
         </td>
      </tr>
      <tr>
         <td><code>STATE_WAIT_ACTION_CASE_CHOSEN</code></td>
         <td>
            <p>The battler's battle controller selected an action, and has since been asked to select a sub-case of that action (e.g. a specific move after selecting the use-move action). We need to wait until they give a response, and then handle it.</p>
            <p>First, we wait until all of the <code>gBattleControllerExecFlags</code> for this battler are cleared, and until the link-exec flags for all battlers are cleared. Then, we respond to <code>gBattleBufferB[gActiveBattler]</code>'s contents differently depending on the action. Nearly all actions advance to <code>STATE_WAIT_ACTION_CONFIRMED_STANDBY</code> when finished, save for a few that kick the battler back to <code>STATE_BEFORE_ACTION_CHOSEN</code> if the controller's choice is invalid, and a few cases where picking a move that isn't allowed (e.g. due to Disable) leads to <code>STATE_SELECTION_SCRIPT</code>.</p>
         </td>
      </tr>
      <tr>
         <td><code>STATE_WAIT_ACTION_CONFIRMED_STANDBY</code></td>
         <td>
            <p>This battler has committed to an action, but (if this is a Link Battle) its owning player may now have to wait on other battlers to commit to their own actions. We'll remain at this state until that has happened, and then move on to <code>STATE_WAIT_ACTION_CONFIRMED</code>. (If this isn't a Link Battle, then we'll effectively only remain in this state for a single frame.)</p>
            <p>We wait until all of the <code>gBattleControllerExecFlags</code> for this battler are cleared, and until the link-exec flags for all battlers are cleared. Then, we emit a <code>CONTROLLER_LINKSTANDBYMSG</code> message (even in local battles) and advance to <code>STATE_WAIT_ACTION_CONFIRMED</code>. The <code>CONTROLLER_LINKSTANDBYMSG</code> message basically implies <code>CONTROLLER_ENDBOUNCE</code>: a controller should do everything it'd do for the latter, along with potentially some extra behaviors as well.</p>
            <p>Nearly all execution paths lead through the "confirmed/standby" state before reaching the "confirmed" state.</p>
         </td>
      </tr>
      <tr>
         <td><code>STATE_WAIT_ACTION_CONFIRMED</code></td>
         <td>
            <p>The battler's action has been confirmed. We wait until all of the <code>gBattleControllerExecFlags</code> for this battler are cleared, and until the link-exec flags for all battlers are cleared, and then we act on this: if, at the end of the function, all battlers test as confirmed, then we execute their actions by advancing <code>gBattleMainFunc</code> to the latent <code>SetActionsAndBattlersTurnOrder</code> function.</p>
         </td>
      </tr>
      <tr>
         <td><code>STATE_SELECTION_SCRIPT</code></td>
         <td>
            <p>The battler needs to run a "selection script" &mdash; that is, a battle script that allows the player to make some sort of choice. Afterwards, we'll advance to action state <code>gBattleStruct->stateIdAfterSelScript[gActiveBattler]</code>.</p>
            <p>Before running the script, we need to wait until all of the <code>gBattleControllerExecFlags</code> for this battler are cleared, and until the link-exec flags for all battlers are cleared. Then, the script is run (from right here, rather than from anywhere else in the battle system).</p>
         </td>
      </tr>
      <tr>
         <td><code>STATE_WAIT_SET_BEFORE_ACTION</code></td>
         <td>
            <p>We wait until all of the <code>gBattleControllerExecFlags</code> for this battler are cleared, and until the link-exec flags for all battlers are cleared, and then advance to <code>STATE_BEFORE_ACTION_CHOSEN</code>.</p>
         </td>
      </tr>
      <tr>
         <td><code>STATE_SELECTION_SCRIPT_MAY_RUN</code></td>
         <td>
            <p>This behaves identically to <code>STATE_SELECTION_SCRIPT</code> but for one thing: after the script completes, we check if <code>gBattleBufferB[gActiveBattler][1] == B_ACTION_NOTHING_FAINTED</code>. If so, then instead of going to the return state, we set the battler's chosen action to <code>B_ACTION_RUN</code> and advance to state <code>STATE_WAIT_ACTION_CONFIRMED_STANDBY</code>. In other words, it doesn't mean "a selection script may run [execute];" it means "a selection script will offer the player a choice to run [flee]."</p>
            <p>This is used for selection scripts that allow the player to forfeit a battle. Specifically: if a controller chooses any action except <code>USE_MOVE</code>, <code>USE_ITEM</code>, <code>SWITCH</code>, <code>SAFARI_BALL</code>, <code>SAFARI_POKEBLOCK</code>, and <code>CANCEL_PARTNER</code>, then we assume they chose to flee. Choosing to run from a trainer battle at the Battle Frontier or Trainer Hill queues <code>BattleScript_AskIfWantsToForfeitMatch</code> to run, and advances to <code>STATE_SELECTION_SCRIPT_MAY_RUN</code> to execute it.</p>
         </td>
      </tr>
   </tbody>
</table>

Further information on `STATE_WAIT_ACTION_CONFIRMED_STANDBY` can be found at this footnote. I can't put it in the table because Markdown doesn't work inside of more complex HTML structures, but paragraphs don't work (well) in GitHub Markdown tables.[^STATE_WAIT_ACTION_CONFIRMED_STANDBY]

[^STATE_WAIT_ACTION_CONFIRMED_STANDBY]: <p>We wait until all of the `gBattleControllerExecFlags` for this battler are cleared, and until the link-exec flags for all battlers are cleared. Then, we emit a `CONTROLLER_LINKSTANDBYMSG` message (even in local battles) and advance to `STATE_WAIT_ACTION_CONFIRMED`.</p><p>The <code>CONTROLLER_LINKSTANDBYMSG</code> message implies the behavior of <code>CONTROLLER_ENDBOUNCE</code>, is only meant to be acted on by the local player controller, and only displays a "Link standby..." textbox when specific parameters are sent. Here, we'll send those parameters to the controller if: this is a Multi Battle or isn't a Double Battle (i.e. is a Single Battle); or the current battler is on the righthand flank; or the current battler is absent. These are the conditions in which a player may finish making all of their selections and now potentially have to wait on other players:</p><ul><li>The player can only end up in control of the right flank if they're in a Double Battle, or if they're in a Link Multi Battle and they aren't the <a href="./battle concepts.md#link-master">link master</a>. Either way, when they commit to a right-flank action, that's the last decision they get to make until the next turn.</li><li>The player can only have a Pokemon absent on the field if: they're in a Double Battle, and all but one of their Pokemon are fainted or unusable in battle; or they're in a Multi Battle, and all of their Pokemon are fainted but their ally is still standing. Absent Pokemon only lead to <code>STATE_WAIT_ACTION_CONFIRMED_STANDBY</code> in Multi Battles; otherwise they skip to <code>STATE_WAIT_ACTION_CONFIRMED</code>. In a Multi Battle, if all of the player's Pokemon are down, then the player has no choices to make.</li></ul><p>So... why don't we see "Link standby..." in local Double Battles? Well, because <code>PrintLinkStandbyMsg</code> checks if this is a link battle before actually displaying anything.</p>

#### `SetActionsAndBattlersTurnOrder`
This function is responsible for determining the order in which battlers' actions should occur. It runs as one of the `gBattleMainFunc` callbacks, but it always runs start to finish: it's not latent, but it could be made so if desired.

It handles the following cases:

* In the Safari Zone, battlers perform their actions in the same order as their battler IDs. I believe in practice, this makes the player always go first.
* All other battles run more nuanced checks...
  * First, we check if either participant in the battle wants to flee (`B_ACTION_RUN`). If so, then that participant is prioritized to perform their action first, and then all other battlers perform their actions in battler ID order.
    * Link Battles check if any participant wants to run. Non-Link Battles only handle the case of battlers 0 or 2 wanting to run. If you want to implement Wild Double Battles, then you'll want to change this, but it should be trivial: just make it so this code always uses the Link Battle logic.
  * If no one wants to run...
    * Battlers who are trying to use an item or switch Pokemon are prioritized.
    * All other battlers are first written into the array in battler ID order, and then sorted based on calls to `GetWhoStrikesFirst`.

Once the function is done, it sets `gBattleMainFunc` to `CheckFocusPunch_ClearVarsBeforeTurnStarts` and sets `gBattleStruct->focusPunchBattlerId` to 0.
  

### Execution of a battle script

Battle scripts can be run from multiple places when necessary. The usual entry point is `BattleScriptExecute`, but sometimes they'll also be run as part of other latent processes, like [`HandleTurnActionSelectionState`](#HandleTurnActionSelectionState). This fact is likely why the battle script engine is so simple (and relies on so many globals), to the point that script commands have to advance their own instruction pointer: all it takes to run a script is to repeatedly grab the byte at the instruction pointer and call into the command table, and you can easily do that from any latent C function.

* `BattleScriptExecute`
  * Set `gBattlescriptCurrInstr` to the location of the script to execute.
  * Push `gBattleMainFunc` onto the `gBattleResources->battleCallbackStack` stack.
  * Set `gBattleMainFunc` to `RunBattleScriptCommands_PopCallbacksStack`.
  * Set `gCurrentActionFuncId` to 0.
* `RunBattleScriptCommands_PopCallbacksStack`
  * If `gCurrentActionFuncId` is either `B_ACTION_TRY_FINISH`[^B_ACTION_TRY_FINISH] or `B_ACTION_FINISHED`[^B_ACTION_FINISHED]:
    * If `gBattleResources->battleCallbackStack` is not empty:
      * Pop the topmost entry.
    * Set `gBattleMainFunc` to the popped entry (or if there isn't one, the zeroth entry) of `gBattleResources->battleCallbackStack`.
  * Else, execute the script (pull the byte at `gBattlescriptCurrInstr`, look it up in the battle script comamnd table, yadda yadda yadda).
  
[^B_ACTION_FINISHED]: `B_ACTION_FINISHED` is written to `gCurrentActionFuncId` to indicate that the current turn action is finished. This happens if `gBattleOutcome` is set to a non-zero value (with this potentially being detected at various points during the turn/action logic), or if the `finishaction` or `finishturn` battle script commands run, with the latter also setting `gCurrentTurnActionNumber` to `gBattlersCount`.

[^B_ACTION_TRY_FINISH]: <p>`B_ACTION_TRY_FINISH` is written to `gCurrentActionFuncId` in response to the script commands `end` and `end2`. Additionally, when any Pokemon tries to switch out while an attacker has used Pursuit on it (and is not asleep, frozen, fainted, or blocked by Truant), `jumpifnopursuitswitchdmg` writes `B_ACTION_TRY_FINISH` as the attacker's action. Switch-outs are always processed before most attacks, so this preempts Pursuit's non-boosted damage. The unused `pursuitdoubles` command also writes `B_ACTION_TRY_FINISH`, likely with similar intent.</p><p>The `B_ACTION_TRY_FINISH` action itself just invokes the latent function `HandleFaintedMonActions`, progressing to `B_ACTION_FINISHED` when that function completes.</p>