#include "lu/utils_item.h"

#include "constants/items.h"

bool8 Lu_ItemIsAPokeBall(u16 global_item_id) {
   if (global_item_id < FIRST_BALL)
      return FALSE;
   if (global_item_id > LAST_BALL)
      return FALSE;
   return TRUE;
}

u8 Lu_GetItemHMNumber(u16 global_item_id) {
   u8 num;
   
   if (global_item_id < ITEM_HM01)
      return 0;
   num = global_item_id - ITEM_HM01 + 1;
   if (num > NUM_HIDDEN_MACHINES)
      return 0;
   return num;
}
u8 Lu_GetItemTMNumber(u16 global_item_id) {
   u8 num;
   
   if (global_item_id < ITEM_TM01)
      return 0;
   num = global_item_id - ITEM_TM01 + 1;
   if (num > NUM_TECHNICAL_MACHINES)
      return 0;
   return num;
}

#include "constants/item_effects.h"
#include "battle.h" // gEnigmaBerries
#include "main.h" // gMain
#include "pokemon.h"

static const u8* GetEffectsOfItem(u16 global_item_id) {
   if (global_item_id < ITEM_POTION)
      return NULL;
   
   if (global_item_id == ITEM_ENIGMA_BERRY) {
      if (gMain.inBattle)
         return gEnigmaBerries[gActiveBattler].itemEffect;
      return gSaveBlock1Ptr->enigmaBerry.itemEffect;
   }
   
   return gItemEffectTable[global_item_id - ITEM_POTION];
}

bool8 Lu_ItemIsARevive(u16 global_item_id) {
   const u8* itemEffect = GetEffectsOfItem(global_item_id);
   if (itemEffect == NULL)
      return FALSE;
   
   if (itemEffect[4] & ITEM4_REVIVE) {
      return TRUE;
   }
   
   return FALSE;
}
bool8 Lu_ItemIsOnlyARevive(u16 global_item_id) {
   const u8* itemEffect = GetEffectsOfItem(global_item_id);
   if (itemEffect == NULL)
      return FALSE;
   
   if (itemEffect[0] || itemEffect[1] || itemEffect[2] || itemEffect[3]) {
      return FALSE;
   }
   if (itemEffect[4] != (ITEM4_REVIVE | ITEM4_HEAL_HP)) {
      return FALSE;
   }
   if (itemEffect[5]) {
      return FALSE;
   }
   
   return TRUE;
}