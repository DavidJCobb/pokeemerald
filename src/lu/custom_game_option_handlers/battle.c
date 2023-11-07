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

static bool8 CurrentAttackerIsPlayerNPCAlly() {
   if (gBattleTypeFlags & BATTLE_TYPE_INGAME_PARTNER) {
      if (GetBattlerPosition(gActiveBattler) == B_POSITION_PLAYER_RIGHT) {
         return TRUE;
      }
   }
   return FALSE;
}

u16 ApplyCustomGameBattleAccuracyScaling(u16 accuracy) {
   u16 scale;
   
   if (!ShouldApplyToThisBattle()) {
      return accuracy;
   }
   scale = gCustomGameOptions.scale_battle_accuracy_enemy;
   if (GetBattlerSide(gActiveBattler) == B_SIDE_PLAYER) {
      scale = gCustomGameOptions.scale_battle_accuracy_player;
      if (CurrentAttackerIsPlayerNPCAlly()) {
         scale = gCustomGameOptions.scale_battle_accuracy_ally;
      }
   }
   return ApplyCustomGameScale_u16(accuracy, scale);
}

void ApplyCustomGameBattleDamageScaling(void) {
   u16 scale;
   
   if (!ShouldApplyToThisBattle()) {
      return;
   }
   scale = gCustomGameOptions.scale_battle_damage_dealt_by_enemy;
   if (GetBattlerSide(gActiveBattler) == B_SIDE_PLAYER) {
      scale = gCustomGameOptions.scale_battle_damage_dealt_by_player;
      if (CurrentAttackerIsPlayerNPCAlly()) {
         scale = gCustomGameOptions.scale_battle_damage_dealt_by_ally;
      }
   }
   gBattleMoveDamage = ApplyCustomGameScale_s32(gBattleMoveDamage, scale);
}