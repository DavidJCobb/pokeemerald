#include "lu/generated/struct-serialize//serialize_ItemSlot.h"

#include "global.h"

#include "lu/bitstreams.h"

void lu_BitstreamRead_ItemSlot(struct lu_BitstreamState* state, struct ItemSlot* v) {
   v->itemId = lu_BitstreamRead_u16(state, 9);
   v->quantity = lu_BitstreamRead_u8(state, 7);
}

void lu_BitstreamWrite_ItemSlot(struct lu_BitstreamState* state, const struct ItemSlot* v) {
   lu_BitstreamWrite_u16(state, v->itemId, 9);
   lu_BitstreamWrite_u8(state, v->quantity, 7);
}
