
# Pending bugfixes

## Catching a Wild Pokemon doesn't allow your Pokemon to evolve

Catching a Wild Pokemon sets the battle outcome to `B_OUTCOME_MON_CAUGHT`. The code which handles potential Pokemon evolution at the end of a battle only checks for `B_OUTCOME_WON`.

We'll want to update `FreeResetData_ReturnToOvOrDoEvolutions` in [`battle_main.c`](/src/battle_main.c) to use the `TryEvolvePokemon` latent function if `gLeveledUpInBattle != 0 && (gBattleOutcome == B_OUTCOME_WON || gBattleOutcome == B_OUTCOME_MON_CAUGHT))`.