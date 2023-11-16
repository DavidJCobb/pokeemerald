#ifndef GUARD_LU_UTILS_ITEM
#define GUARD_LU_UTILS_ITEM

#include "global.h"

bool8 Lu_ItemIsAPokeBall(u16 global_item_id);

u8 Lu_GetItemHMNumber(u16 global_item_id); // 0 == not a HM
u8 Lu_GetItemTMNumber(u16 global_item_id); // 0 == not a TM

bool8 Lu_ItemIsARevive(u16 global_item_id); // TRUE if any of the item's effects would revive
bool8 Lu_ItemIsOnlyARevive(u16 global_item_id); // TRUE if the item's only effect is to revive

#endif