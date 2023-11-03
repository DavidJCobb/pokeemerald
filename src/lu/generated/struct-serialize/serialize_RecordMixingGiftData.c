#include "lu/generated/struct-serialize//serialize_RecordMixingGiftData.h"

#include "global.h"

#include "lu/bitstreams.h"

void lu_BitstreamRead_RecordMixingGiftData(struct lu_BitstreamState* state, struct RecordMixingGiftData* v) {
   u8 i;
   v->unk0 = lu_BitstreamRead_u8(state, 8);
   v->quantity = lu_BitstreamRead_u8(state, 8);
   v->itemId = lu_BitstreamRead_u16(state, 9);
   for (i = 0; i < 8; ++i) {
      v->filler4[i] = lu_BitstreamRead_u8(state, 8);
   }
}

void lu_BitstreamWrite_RecordMixingGiftData(struct lu_BitstreamState* state, const struct RecordMixingGiftData* v) {
   u8 i;
   lu_BitstreamWrite_u8(state, v->unk0, 8);
   lu_BitstreamWrite_u8(state, v->quantity, 8);
   lu_BitstreamWrite_u16(state, v->itemId, 9);
   for (i = 0; i < 8; ++i) {
      lu_BitstreamWrite_u8(state, v->filler4[i], 8);
   }
}
