#ifndef GUARD_LU_CUSTOM_GAME_OPTION_HANDLERS
#define GUARD_LU_CUSTOM_GAME_OPTION_HANDLERS
#include "global.h"

u16 ApplyCustomGameBattleAccuracyScaling(u16);
void ApplyCustomGameBattleDamageScaling(void); // modifies gBattleMoveDamage

u32 ApplyCustomGameBattleMoneyVictoryScaling(u32);

bool8 CustomGamesAllowBattleBackfieldHealing();
bool8 CustomGamesAllowRevivesInBattle();

#endif