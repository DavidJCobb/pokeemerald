
# Areas of interest

## Battles

### Catching Pokemon

* `Cmd_handleballthrow` in `battle_script_commands.c` handles:
** Setting `gBattlerTarget` (readable in battle scripts as `BS_TARGET`) to the Pokemon to catch
** Applying catch rate modifiers for special Poke Balls, e.g. the Net Ball
** Applying catch rate modifiers for status effects
** Applying the catch rate formula generally
** Computing whether the catch is successful, and jumping to specific battle scripts for catching if so or if not. For example, script `BattleScript_SuccessBallThrow` handles the entire UI flow after a successful catch, including what messages are shown in what order, whether the player gets a chance to nickname the caught Pokemon, and so on.
** Setting the caught Pokemon's Poke Ball

If we wanted to support wild Double Battles, where the player can choose what Pokemon to catch, then we'd have to make the following edits...

* `CompleteWhenChoseItem` in `battle_controller_player.c` would need to check whether the used item is a Poke Ball, and whether the player is allowed to catch Wild Pokemon right now (i.e. not in a trainer battle, cutscene battle, etc.). If so, then it'd need to let the player select a ball target (and gracefully handle the player cancelling).
* `CompleteWhenChoseItem` stores the selected item to use by writing it into one of two buffers used to store selected action state &mdash; in this case, the first uint16_t of `BUFFER_B`. Meanwhile, `HandleInputChooseTarget` is used to handle choosing a move target, and stores its results by writing them into the first three bytes of `BUFFER_B`, writing the integral constant 10, the move selection cursor position (I assume this is a move index from 0 to 3), and `gMultiUsePlayerCursor` which I assume is the targeted battler. If we could find a way to combine the selected item ID and the cursor position...
 * Bear in mind that `HandleInputChooseTarget` requires some boilerplate before being called: [you must set the cursor's initial position](https://github.com/DavidJCobb/pokeemerald/blob/843830374ca2b56d042a680ce04925d1b3164de6/src/battle_controller_player.c#L531) based on what battlers are on the field. Note also that it doesn't constrain cursor movement, so without changes, you'd be able to toss Poke Balls at your own Pokemon.
* When it comes time for the game to actually process a battler's selected action, it can read the move target as `gBattleBufferB[gActiveBattler][3]`.
* `Cmd_getexp` changes the background music after fainting a Wild Pokemon.
* What about Recorded Battles? Some of the stats tracked by `Cmd_handleballthrow`, like the number of catch attempts, rely on the invariant that you only battle one Wild Pokemon at a time and the battle ends when a catch is successful.