
# `BattleStruct`

This data structure holds the bulk of all state related to battles. Its sole instance is `gBattleStruct`.

## Members

| Type | Name | Summary |
| :- | :- | :- |
| `u8` | `turnEffectsTracker` |
| `u8` | `turnEffectsBattlerId` |
| `u8` | `unused_0` |
| `u8` | `turnCountersTracker` |
| `u8[]` | `wrappedMove` | "Leftover from Ruby's ewram access." |
| `u8[]` | `moveTarget` |
| `u8` | `expGetterMonId` |
| `u8` | `unused_1` |
| `u8` | [`wildVictorySong`](#wildvictorysong) | Used to track whether the battle's background music has been changed to `MUS_VICTORY_WILD` yet. |
| `u8` | `dynamicMoveType` |
| `u8[]` | [`wrappedBy`](#wrappedby) | Indicates what battlers are the victims of Wrap used by what other battlers. |
| `u16[]` | `assistPossibleMoves` |
| `u8` | `focusPunchBattlerId` |
| `u8` | `battlerPreventingSwitchout` |
| `u8` | [`moneyMultiplier`](#moneymultiplier) |
| `u8` | `savedTurnActionNumber` |
| `u8` | `switchInAbilitiesCounter` |
| `u8` | `faintedActionsState` |
| `u8` | `faintedActionsBattlerId` |
| `u16` | `expValue` | State for the [amortized](./battle%20concepts.md#amortized) `getexp` script command: the amount of EXP currently being earned by a Pokemon. |
| `u8` | [`scriptPartyIdx`](#scriptpartyidx) | A [battler ID](./battle%20concepts.md#battler-id) used when printing a battler's (nick)name, to tell whether they should be described as "Wild"/"Foe" or not. |
| `u8` | `sentInPokes` |
| `bool8[]` | `selectionScriptFinished` |
| `u8[]` | `battlerPartyIndexes` |
| `u8[]` | `monToSwitchIntoId` |
| `u8[]` | `battlerPartyOrders` |
| `u8` | `runTries` | The number of times the player has tried to run from the current battle (`TryRunFromBattle`). Influences escape odds. Can overflow. |
| `u8[]` | `caughtMonNick` | State for the [amortized](./battle%20concepts.md#amortized) `trygivecaughtmonnick` function: the nickname the player is currently assigning to a caught Wild Pokemon. |
| `u8` | `unused_2` |
| `u8` | `safariGoNearCounter` |
| `u8` | `safariPkblThrowCounter` |
| `u8` | `safariEscapeFactor` |
| `u8` | `safariCatchFactor` |
| `u8` | `linkBattleVsSpriteId_V` | Sprite ID for the letter "V" |
| `u8` | `linkBattleVsSpriteId_S` | Sprite ID for the letter "S" |
| `u8` | [`formToChangeInto`](#formtochangeinto) | A form for a Castform to change into, briefly stored here so it can be juggled across execution contexts (i.e. from ability effect handlers to the code backing a specific battle script command that they induce to run). |
| `u8[]` | `chosenMovePositions` |
| `u8[]` | `stateIdAfterSelScript` |
| `u8[]` | `unused_3` |
| `u8` | `prevSelectedPartySlot` |
| `u8[]` | `unused_4` |
| `u8` | `stringMoveType` | Global state relied upon by `BufferStringBattle(u16 stringID)` in [`battle_message.c`](/src/battle_message.c): a type value (e.g. `TYPE_PSYCHIC`). |
| `u8` | `expGetterBattlerId` | State for the [amortized](./battle%20concepts.md#amortized) `getexp` script command: the [battler ID](./battle%20concepts.md#battler-id) of the Pokemon currently earning EXP. |
| `u8` | `unused_5` |
| `u8` | `absentBattlerFlags` |
| `u8` | `palaceFlags` | "First 4 bits are 'is <= 50% HP and not asleep' for each battler, last 4 bits are selected moves to pass to AI" |
| `u8` | `field_93` | "related to choosing pokemon?" |
| `u8` | `wallyBattleState` |
| `u8` | `wallyMovesState` |
| `u8` | `wallyWaitFrames` |
| `u8` | `wallyMoveFrames` |
| `u8[]` | `lastTakenMove` | "Last move that a battler was hit with. This field seems to erroneously take 16 bytes instead of 8." |
| `u16[]` | `hpOnSwitchout` |
| `u32` | `savedBattleTypeFlags` |
| `u8` | `abilityPreventingSwitchout` |
| `u8` | `hpScale` |
| `u8` | `synchronizeMoveEffect` |
| `bool8` | `anyMonHasTransformed` |
| `void(*)()` | `savedCallback` |
| `u16[]` | `usedHeldItems` |
| `u8[]` | `chosenItem` | "why is this an u8?" |
| `u8[]` | `AI_itemType` |
| `u8[]` | `AI_itemFlags` |
| `u16[]` | `choicedMove` |
| `u16[]` | `changedItems` |
| `u8` | `intimidateBattler` | The [battler ID](./battle%20concepts.md#battler-id) of the Pokemon whose Intimidate ability effect is currently activating. This is set by the handlers for `ABILITYEFFECT_INTIMIDATE1` and `ABILITYEFFECT_INTIMIDATE2`, and influences the behavior of the `trygetintimidatetarget` script command. |
| `u8` | `switchInItemsCounter` |
| `u8` | `arenaTurnCounter` |
| `u8` | `turnSideTracker` |
| `u8[]` | `unused_6` |
| `u8` | `givenExpMons` | "Bits for enemy party's Pokemon that give exp to player's party." |
| `u8[]` | `lastTakenMoveFrom` | "a 3-D array [target][attacker][byte]" |
| `u16[]` | `castformPalette` |
| `union` | [`multiBuffer`](#multibuffer) | Used to synchronize basic information at the start of a Link Battle. |
| `u8` | `wishPerishSongState` |
| `u8` | `wishPerishSongBattlerId` |
| `bool8` | [`overworldWeatherDone`](#overworldweatherdone) | Seemingly meant to track when weather from the overworld is replaced by weather triggered in battle, but it's likely incomplete. |
| `u8` | [`atkCancellerTracker`](#atkcancellertracker) | Seems like someone used a global variable for something that should've been local, to make debugging easier. |
| `struct BattleTvMovePoints` | `tvMovePoints` |
| `struct BattleTv` | `tv` |
| `u8[]` | `unused_7` |
| `u8[]` | `AI_monToSwitchIntoId` |
| `s8[]` | `arenaMindPoints` |
| `s8[]` | `arenaSkillPoints` |
| `u16[]` | `arenaStartHp` |
| `u8` | `arenaLostPlayerMons` | "Bits for party member, lost as in referee's decision, not by fainting." |
| `u8` | `arenaLostOpponentMons` |
| `u8` | `alreadyStatusedMoveAttempt` | "As bits for battlers; For example when using Thunder Wave on an already paralyzed Pokémon. |


### `atkCancellerTracker`

The term "attack canceller" seems to have been chosen for the various factors that can cancel the use of a move, such as being asleep, or being subject to the effects of the Truant ability.

The `atkCancellerTracker` value is set to zero when a battler attempts to use a move (`HandleAction_UseMove` in `battle_util.c`), and then used as a loop index (where one would typically use a local variable instead) within `AtkCanceller_UnableToUseMove`, which checks whether any of the cancel conditions are met and if so, executes them.

The variable doesn't appear to be read from anywhere else, and it doesn't seem like it would ever be non-zero (which would skip potential cancellation causes) by the time cancellations are checked. This seems like a situation where a local variable was made global instead for some reason (perhaps as a debugging aid?).


### `formToChangeInto`

A castform form value (`CASTFORM_`) as defined in [`constants/battle.h`](/include/constants/battle.h), potentially OR'd with `CASTFORM_SUBSTITUTE`.

The value is first set up in `AbilityBattleEffects` (in [`battle_util.c`](/src/battle_util.c)) by the handlers for the Forecast and Air Lock abilities, which also queue the `BattleScript_CastformChange` battle script to run. Then, the `docastformchangeanimation` [battle script command](/src/battle_script_commands.c) adds the Substitute flag if necessary, and passes the value as an argument to `BtlController_EmitBattleAnimation`.

In short: this variable is only here so it can be briefly juggled across different execution contexts.


### `moneyMultiplier`

Used to scale the player's monetary winnings from singleplayer battles. It's initialized to 1 in `BattleStartClearSetData`[^BattleStartClearSetData], and will be set to 2 if the player switches in a Pokemon holding an Amulet Coin.

The Amulet Coin effect is enforced by `ItemBattleEffects` in `battle_util.c`, which executes held item effects in response to specific events.


### `multiBuffer`

This appears to be a buffer used to transmit link data to other players at the start of the battle. It's a union between a `LinkBattlerHeader` and a `u32 battleVideo[2]`.

The link battler header only contains the following information:

* A little-endian version signature for link communications.
* A little-endian flags-mask for the V.S. screen shown at the start of battle.
* A `BattleEnigmaBerry` instance, presumably used to send the local player's Enigma Berry effects to other players. (Each player has a unique Enigma Berry effect tracked in `gEnigmaBerries`; players aren't forced to use the Link Master's[^dfn-link-master] berry effect.) This is initialized by `SetPlayerBerryDataInBattleStruct()`[^SetPlayerBerryDataInBattleStruct].

The `battleVideo` dwords, meanwhile, are also sent during the start of a multiplayer battle (`CB2_HandleStartMultiBattle` phase 8), being set to `gBattleTypeFlags` and `gRecordedBattleRngSeed`. A code comment in that function claims that the latter value "overwrites berry data" and is undefined behavior. This seems incorrect to me: the two pieces of data are sent at different phases in `CB2_HandleStartMultiBattle`, and as far as I can discern, the Enigma Berry data only overlaps these fields while it's sent, not when it's being received and remembered.


[^dfn-link-master]: <dfn>Link Master</dfn>: In Halo games and on the Xbox platform generally, this would be called the "Party Leader."

[^SetPlayerBerryDataInBattleStruct]: `SetPlayerBerryDataInBattleStruct()` is defined in [`battle_main.c`](/src/battle_main.c).


### `overworldWeatherDone`

Seemingly intended to track when any weather effect inherited from the overworld has been replaced with a new weather effect triggered in battle. In practice, the functionality doesn't seem complete, and nothing seems to actually do anything with this variable.

This variable is reset to `FALSE` whenever any player is introduced (i.e. sending out Pokemon for the first time in a battle). Then, in `TryDoEventsBeforeFirstTurn` at the start of the battle, if this variable is `FALSE` (which it must be, surely?) and any Pokemon currently on the field has a switch-in effect that triggers a weather condition, then this variable is set to `TRUE`.


### `scriptPartyIdx`

This is a [battler ID](./battle%20concepts.md#battler-id) used when printing a Pokemon's (nick)name, to tell whether it should be prefixed with "Wild"/"Foe" or not.

The `BattleStringExpandPlaceholders` function (defined in [`src/battle_message.c`](/src/battle_message.c)) takes two arguments, both strings: a source and a destination. It doesn't have access to any of the contextual information that gets passed around by other text-related functions in the battle engine, so it has to rely on global state instead. `gBattleStruct->scriptPartyIndex` is one such piece of global state.


### `wildVictorySong`

Reset to 0 at the start of the battle by `BattleStartClearSetData`, and then managed by the `getexp` [battle script command](/src/battle_script_commands.c).

The `getexp` command is a state machine: it relies on a counter (stored in `gBattleScripting.getexpState`) to amortize its work across multiple frames. On frame 2, it checks if the current battle is a Wild battle, if [battler ID](./battle%20concepts.md#battler-id) 0 isn't fainted, and if `wildVictorySong` is false. If so, then it calls `BattleStopLowHPSound()`, sets the current background music to `MUS_VICTORY_WILD`, and increments `wildVictorySong` to true.

It shouldn't be possible for `getexp` state 2 to run a second time except in a Double Battle, but Wild Double Battles aren't implemented. If hypothetically they were, then the `wildVictorySong` variable would prevent `getexp` from accidentally restarting the music when you faint the second Wild Pokemon. Someone at Game Freak may have been planning ahead.


### `wrappedBy`

An array where both the keys and values are [battler IDs](./battle%20concepts.md#battler-id), used to indicate which Pokemon are victims of the move Wrap, and whom their attackers are.

The relationship between key-battler and value-battler is as such: the key-battler is being wrapped by the value-battler. Additionally, the key-battler will have [`STATUS2_WRAPPED`](./battle%20concepts.md#status2).


