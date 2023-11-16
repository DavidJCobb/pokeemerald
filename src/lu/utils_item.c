#include "lu/utils_item.h"

#include "constants/items.h"

//
// THINGS THAT ARE NEEDED TO ALLOW DISCONTIGUOUS POKE BALL ITEM IDS:
//
//  - A function that can map them to an alternate, contiguous, ID space.
//
//     - An ID space like this already exists for Poke Ball animations; see 
//       notes below. However, there is no "none" ID.
//
//     - An ID space like this would also be useful for serializing the Bag's 
//       Poke Ball pocket into an even more compact bitpacked representation, 
//       so long as the ID space in question includes a "none" value.
//
//  - Everything that relies on a list/enum of Poke Ball item IDs needs to use 
//    the mapping:
//
//     - MON_DATA_POKEBALL
//
//     - All item definitions for Poke Balls, which co-opt the `type` field. This 
//       will be especially messy as constant evaluation, uh, isn't a thing.
//
//     - `ItemIdToBallId` in `battle_anim.h`
//
//     - BattleResults::catchAttempts
//
// AN ALTERNATIVE APPROACH:
//
//    What if we just redefine the item constants so that, for example, ITEM_POTION 
//    is (LAST_BALL + 1) instead of 13?
//
//    Sadly, we can't just place everything in an enum, because enum members aren't 
//    usable in preprocessor directives (they're treated as undefined macros and so 
//    always resolve to 0).
//
// GENERAL CONSIDERATIONS FOR ADDING NEW POKE BALLS:
//
//  - PokemonSubstruct3::pokeball stores the ball a Pokemon was caught in, and is 
//    only 4 bits. Given how Pokerus works, we could reclaim 3 bits from it for 
//    use elsewhere if we rewrite Pokerus.
//
// ================================================================================
//
// An interesting note:
//
//   Function `ItemIdToBallId` in `battle_anim_throw.c` maps item ID constants to 
//   localized ball IDs e.g. BALL_MASTER and BALL_POKE. The localized ID constants 
//   are pulled from an enum in `pokeball.h`, which also defines POKEBALL_COUNT 
//   and is apparently used for all of the many places where Poke Ball graphics 
//   are needed (e.g. sending out Pokemon; the a Pokemon's ball in its Summary 
//   screen; etc.).
//
//   A great many of the graphics and graphics parameters in `battle_anim_throw.c` 
//   use this BALL_* enum as array indices.
//
// Another note:
//
//   BattleResults::catchAttempts in `battle.h` tracks wild Pokemon catch attempts 
//   by ball type (not counting the Master ball for obvious reasons; that gets a 
//   separate bitpacked bool). This array uses the POKEBALL_COUNT value defined in 
//   `pokeball.h`, so adding more ball types will make `BattleResults` larger.
//
//   Each counter is a u8 and is capped at 255, with code to guard against overflow. 
//   It seems like the only place this specific value is used is in the code for TV 
//   shows, and even there, the game only really cares about the total balls thrown 
//   (or whether any balls were thrown at all), not the balls of each type.
//
//   How exactly is this struct used? Game Freak bitpacked some of the fields, so 
//   do we want to avoid making it larger? Is the struct ever sent or stored for 
//   other players e.g. as part of Record Mixing, or is it purely local state to be 
//   made available to scripts and systems that run post-battle?
//
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