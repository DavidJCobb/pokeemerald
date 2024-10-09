
## Battle concepts

### Battle controller

A <dfn>battle controller</dfn> is a trainer participating in an ongoing battle. Battle controllers: are placed in charge of [battler positions](#battler-position) as appropriate; receive *commands*, such as "choose an action from the fight/PKMN/bag/run menu" or "play a Pokemon's fainting cry animation;" and perform actions in response to those commands. Battle controllers are a key element in the abstractions that allow the same battle engine to handle real-time singleplayer battles, linked multiplayer battles, cutscene battles like Wally's capture of Ralts, and recorded battles. The battle engine doesn't have to know or care *how* it's being given commands &mdash; whether they're coming from the player, from an NPC, from over a multiplayer link, or what. The battle engine just ferries command data to where it needs to go and invokes the battle controller function pointer for each [battler](#battler-id) on the field.

For more detailed information, see [the dedicated page](./battle%20controllers.md).


### Battler ID

A `u8` identifying a battler currently on the field. Battler IDs are always in the range \[0, `MAX_BATTLERS_COUNT`) i.e. \[0, 4) and are indices into the `gBattleMons` array. Battler IDs are functionally equivalent to [battler positions](#battler-position).

Generally speaking, `0xFF` is used to indicate "none" wherever "no battler" is an appropriate value for a battler ID.


### Battler position

The battlefield is divided into four total positions for combatants:

| Constant | Value | Value (binary) |
| :-: | -: | -: |
| `B_POSITION_PLAYER_LEFT` | 0 | `0b00` |
| `B_POSITION_OPPONENT_LEFT` | 1 | `0b01` |
| `B_POSITION_PLAYER_RIGHT` | 2 | `0b10` |
| `B_POSITION_OPPONENT_RIGHT` | 3 | `0b11` |

The least-significant bit of a position represents a <dfn>side</dfn> of the battlefield. `B_SIDE_PLAYER` (0) is the local player's side of the battlefield, and `B_SIDE_OPPONENT` (1) is the opposite side of the field.

The next bit of the position represents the left (`B_FLANK_LEFT` == 0) or right (`B_FLANK_RIGHT` == 1) flank.

Note that "left" and "right" are described relative to each side. The opponent is facing you, so their left-side battler is drawn on the righthand side of the screen, and vice versa.


### Hit markers

A set of flags that are maintained globally, with some being scoped to the current turn; others, the current action; others, the rest of a battle.

All hitmarkers are cleared at the start of a battle.

| Constant | Description | Lifecycle |
| :- | :- | :- |
| `HITMARKER_WAKE_UP_CLEAR` |
| `HITMARKER_IGNORE_BIDE` | Prevents the next invocation of the `datahpupdate` script command from recording damage taken by a Pokemon who has used Bide; this prevents that damage from increasing Bide's eventual attack power. | Set at the start of `DoBattlerEndTurnEffects` and cleared at its end; same for `HandleWishPerishSongOnTurnEnd`. Both of these functions may end up executing battle scripts. Cleared after the `datahpupdate` script command checks it, too. |
| `HITMARKER_DESTINYBOND` |
| `HITMARKER_NO_ANIMATIONS` |
| `HITMARKER_IGNORE_SUBSTITUTE` | Forces invocations of the `hitanimation` and `healthbarupdate` script commands to act as though the relevant battler has no Substitute. Forces the next invocation of the `datahpupdate` script command to act as though the target battler has no active Substitute, after which the hitmarker is cleared. | <p>Set by the `setsubstitute` script command at the same time as `STATUS2_SUBSTITUTE`, presumably to ensure that the substitute doesn't absorb the 25% HP sacrificial damage required to create it in the first place.</p><p>Cleared after the `datahpupdate` script command checks it, or else by `HandleAction_NothingIsFainted` and `HandleAction_ActionFinished`.</p> |
| `HITMARKER_NO_ATTACKSTRING` | | Cleared by `RunTurnActionsFunctions` when an action has finished executing. |
| `HITMARKER_ATTACKSTRING_PRINTED` |
| `HITMARKER_NO_PPDEDUCT` |
| `HITMARKER_SWAP_ATTACKER_TARGET` | Used by the battle script engine to track whether `gBattlerAttacker` and `gBattlerTarget` are currently swapped as a result of running the `swapattackerwithtarget` script command an odd number of times. The hitmarker is checked for and the swap reverted by the `moveend` script command (in the `MOVEEND_UPDATE_LAST_MOVES` step of latent execution). | Set or cleared by the `swapattackerwithtarget` script command, and cleared by the `moveend` script command. |
| `HITMARKER_STATUS_ABILITY_EFFECT` |
| `HITMARKER_SYNCHRONIZE_EFFECT` |
| `HITMARKER_RUN` | Prevents `CheckFocusPunch_ClearVarsBeforeTurnStarts` from executing `BattleScript_FocusPunchSetUp`: battlers who have committed to using Focus Punch will no longer have show the charge-up animation and text at the start of the turn. The hitmarker doesn't seem to be cleared during battle, so this effect is indefinite. | <p>Set on an attempt to run[^hitmarker-run-when], before the battle engine has decided whether the attempt should succeed &mdash; specifically, by `HandleTurnActionSelectionState` when handling `B_ACTION_RUN`, `B_ACTION_SAFARI_RUN`, or a decision to forfeit the match.</p><p>Never cleared during a battle.</p> |
| `HITMARKER_IGNORE_ON_AIR` | Forces the next invocation of `AccuracyCalcHelper` (helper function for the `accuracycheck` script command) to ignore the fact of an attack's target having [`STATUS3_ON_AIR`](#status3). | <p>Set by move effects' battle scripts as needed (e.g. `BattleScript_EffectTwister`).</p><p>Cleared by `AccuracyCalcHelper` after it's checked, or else by `HandleAction_NothingIsFainted` and `HandleAction_ActionFinished`.</p> |
| `HITMARKER_IGNORE_UNDERGROUND` | Forces the next invocation of `AccuracyCalcHelper` (helper function for the `accuracycheck` script command) to ignore the fact of an attack's target having [`STATUS3_UNDERGROUND`](#status3). | <p>Set by move effects' battle scripts as needed (e.g. `BattleScript_EffectEarthquake`).</p><p>Cleared by `AccuracyCalcHelper` after it's checked, or else by `HandleAction_NothingIsFainted` and `HandleAction_ActionFinished`.</p> |
| `HITMARKER_IGNORE_UNDERWATER` | Forces the next invocation of `AccuracyCalcHelper` (helper function for the `accuracycheck` script command) to ignore the fact of an attack's target having [`STATUS3_UNDERWATER`](#status3). | <p>Set by move effects' battle scripts as needed (`BattleScript_EffectHit` handles Surf and `BattleScript_EffectTrap` handles Whirlpool).</p><p>Cleared by `AccuracyCalcHelper` after it's checked, or else by `HandleAction_NothingIsFainted` and `HandleAction_ActionFinished`.</p> |
| `HITMARKER_UNABLE_TO_USE_MOVE` | One of the conditions that prevents the `moveend` script command from allowing a move to hit multiple opposing Pokemon during a Double Battle (in the `MOVEEND_NEXT_TARGET` step of latent execution). | <p>Set by any of: `AtkCanceller_UnableToUseMove`; the `attackcanceler` script command, if the Pokemon is fainted; or `IsMonDisobedient`, if a traded Pokemon disobeys and hits itself.</p><p>Cleared by `RunTurnActionsFunctions` when an action has finished executing, or by `BattleTurnPassed` at the end of a turn.</p> |
| `HITMARKER_PASSIVE_DAMAGE` | Prevents the `datahpupdate` script command from recording damage dealt as part of the Shell Bell's hold item effect (which restores an amount of health equal to one eighth the damage dealt by the holder). | <p>Cleared by `HandleAction_NothingIsFainted`, `HandleAction_ActionFinished`, `RunTurnActionsFunctions` when all turn actions have finished executing, and `BattleTurnPassed`. They *really* wanted to be sure they cleared this one, huh?</p> |
| `HITMARKER_DISOBEDIENT_MOVE` | Never read by anything. Indicates whether any Pokemon disobeyed its trainer during a battle specifically by picking a random move. | <p>Set by `IsMonDisobedient` if it induces a traded Pokemon to disobey by performing a random move.</p><p>Never cleared during a battle.</p> |
| `HITMARKER_PLAYER_FAINTED` | Never read by anything. Indicates that one of the player's Pokemon fainted during the current turn. | <p>Set as applicable by the `tryfaintmon` script command. Set by the `various` script command (sub-commands `VARIOUS_ARENA_PLAYER_MON_LOST` and `VARIOUS_ARENA_BOTH_MONS_LOST`).</p><p>Cleared by `BattleTurnPassed`, and by the `various` script command (sub-command `VARIOUS_RESET_PLAYER_FAINTED`).</p> |
| `HITMARKER_ALLOW_NO_PP` |
| `HITMARKER_GRUDGE` |
| `HITMARKER_OBEYS` | The `setminimize` script command only applies [`STATUS3_MINIMIZED`](#status3) if this hitmarker is set. `BattleArena_AddSkillPoints` uses this hitmarker to tell whether the battler it's being invoked for should gain skill points. The `moveend` script command uses this hitmarker for various purposes, including as part of `MOVEEND_CHOICE_MOVE` (to update the Choice Band move), `MOVEEND_UPDATE_LAST_MOVES`, and `MOVEEND_MIRROR_MOVE`. | <p>Set by the `attackcanceler` script command if the Pokemon has [`STATUS2_MULTIPLETURNS`](#status2) or else if `IsMonDisobedient()` doesn't cause the Pokemon to disobey.</p><p>Cleared by `HandleAction_NothingIsFainted` and `HandleAction_ActionFinished`.</p> |
| `HITMARKER_NEVER_SET` | Unused. |
| `HITMARKER_CHARGING` |
| `HITMARKER_FAINTED` |

[^hitmarker-run-when]: Notably, `HITMARKER_RUN` is set when any battle controller commits to a decision to run. If, hypothetically, Wild Double Battles existed, it would be possible for the player to order their left-flank Pokemon to run &mdash; setting `HITMARKER_RUN` &mdash; and then cancel that order by backing out with their right-flank Pokemon, without the hitmarker being reset back to its initial state.


### Status

There are three different status enums, all defined in [`constants/battle.h`](/include/constants/battle.h).

#### `STATUS1`: Persistent status conditions
<a name="status1"></a>

Pokemon that attain these status conditions will continue to possess them in some form after switching out, and after the battle ends. During battle, they're stored in `BattlePokemon::status1`.

| Constant | Bitcount | Bits | Description |
| :- | -: | -: | :- |
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
| `STATUS2_ESCAPE_PREVENTION` | 1 | `0b00000100000000000000000000000000` | Indicates that this Pokemon is being prevented from leaving the battlefield by another battler. Refer to [`gDisableStructs[this_battler].battlerPreventingEscape`](./battle%20globals.md#gdisablestructs) for that battler's identity. |
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
| `STATUS3_ALWAYS_HITS`       | 2 | `0b000000000000000011000` | A countdown to an imminent attack which will be guaranteed to hit this battler. Refer to [`gDisableStructs[this_battler].battlerWithSureHit`](./battle%20globals.md#gdisablestructs) for the attacker's identity. |
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

### Latent functions
<a name="latent"></a>

Several hardcoded tasks and script commands distribute their computation over multiple frames, instead of running start-to-finish in one shot. Sometimes, this is done because a function or process must wait on events occurring elsewhere, such as UI interactions or link connectivity. Other times, though, code that could easily just run start-to-finish is instead built to distribute its computation across frames, presumably to avoid potential performance issues &mdash; frame rate lag, audio stutters, and the like.

The general pattern is this: the latent function is built as a switch-case statement, switching on a "state" variable of some sort. The variable is typically a counter, being incremented at the end of each case, though this won't be the case when branching execution is needed. The latent function, then, is called once per frame until its work completes. Any variables that are needed across multiple stages of computation (including the "state" variable itself) can't be local (because they'll be lost at the end of each frame) and must be stored elsewhere.

As for *how* a function gets itself to be called once per frame, there are two common cases:

* Some processes are implemented as frame callback handlers (the functions whose names are prefixed with `CB2`).
* For battle scripts, the script instruction pointer doesn't advance automatically; battle script commands must advance it manually. If they don't advance the instruction pointer, then they'll just run again on the next frame.

It's useful to have a piece of jargon that instantly refers to this one specific pattern and not anything else in the codebase, and as it happens, there's a term from other game engines (such as Unreal and the Creation Engine) that maps nicely: <dfn>latent functions</dfn>. Latent functions don't run start-to-finish immediately, but rather may perform operations over time before eventually returning to whatever could be considered their "caller" (e.g. the containing script, if the latent function is a script command).

Examples include but are most certainly not limited to...

* There are several functions that set up the initial state for battles, depending on the battle type (Link Battle, Multi Battle, etc.). These amortize their work over multiple frames and generally use `gBattleCommunication[MULTIUSE_STATE]` as their state variable.
* The `getexp` battle script command amortizes its work over multiple frames, possibly to avoid lag. It uses `gBattleScripting.getexpState` as its state variable, and uses `gBattleStruct->expValue` and `gBattleStruct->expGetterBattlerId` to track data that gets used across multiple stages of computation.
* The `trygivecaughtmonnick` battle script command does its work over multiple frames, because it has to wait on GUI events and interactions. It uses `gBattleCommunication[MULTIUSE_STATE]` as its state variable, and relies on `gBattleStruct->caughtMonNick` to hold the player's chosen nickname for a caught Wild Pokemon.


