#include "lu/custom_game_option_handlers/battle.h"
#include "lu/custom_game_options.h"

#include "constants/battle.h"
#include "battle.h"
#include "battle_anim.h" // GetBattlerPosition

//
// TODO: How do we ensure that this doesn't break recorded battles? Are we going to 
// have to store what the custom game options were at the time a battle was recorded?
//

static bool8 ShouldApplyToThisBattle() {
   if (gBattleTypeFlags & (
      BATTLE_TYPE_LINK | 
      BATTLE_TYPE_FRONTIER | 
      BATTLE_TYPE_EREADER_TRAINER | 
      BATTLE_TYPE_RECORDED_LINK | 
      BATTLE_TYPE_WALLY_TUTORIAL
   )) {
      return FALSE;
   }
   return TRUE;
}

enum {
   PLAYER,
   ALLY,
   ENEMY,
};
//
static u8 IdentifyCurrentAttacker() {
   u8 pos = GetBattlerPosition(gBattlerAttacker);
   if ((pos & BIT_SIDE) != B_SIDE_PLAYER) {
      return ENEMY;
   }
   if (gBattleTypeFlags & BATTLE_TYPE_INGAME_PARTNER) {
      if (pos == B_POSITION_PLAYER_RIGHT) {
         return ALLY;
      }
   }
   return PLAYER;
}

u16 ApplyCustomGameBattleAccuracyScaling(u16 accuracy) {
   u16 scale;
   
   if (!ShouldApplyToThisBattle()) {
      return accuracy;
   }
   switch (IdentifyCurrentAttacker()) {
      case ENEMY:
         scale = gCustomGameOptions.scale_battle_accuracy_enemy;
         break;
      case PLAYER:
         scale = gCustomGameOptions.scale_battle_accuracy_player;
         break;
      case ALLY:
         scale = gCustomGameOptions.scale_battle_accuracy_ally;
         break;
   }
   return ApplyCustomGameScale_u16(accuracy, scale);
}

void ApplyCustomGameBattleDamageScaling(void) {
   u16 scale;
   
   if (!ShouldApplyToThisBattle()) {
      return;
   }
   switch (IdentifyCurrentAttacker()) {
      case ENEMY:
         scale = gCustomGameOptions.scale_battle_damage_dealt_by_enemy;
         break;
      case PLAYER:
         scale = gCustomGameOptions.scale_battle_damage_dealt_by_player;
         break;
      case ALLY:
         scale = gCustomGameOptions.scale_battle_damage_dealt_by_ally;
         break;
   }
   gBattleMoveDamage = ApplyCustomGameScale_s32(gBattleMoveDamage, scale);
}

u32 ApplyCustomGameBattleMoneyVictoryScaling(u32 reward) {
   return ApplyCustomGameScale_u32(reward, gCustomGameOptions.scale_player_money_gain_on_victory);
}