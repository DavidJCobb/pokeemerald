#include "lu/generated/struct-serialize//serialize_RecordMixingGift.h"

#include "global.h"

#include "lu/bitstreams.h"

// dependencies
#include "lu/generated/struct-serialize//serialize_RecordMixingGiftData.h"

void lu_BitstreamRead_RecordMixingGift(struct lu_BitstreamState* state, struct RecordMixingGift* v) {
   v->checksum = lu_BitstreamRead_s32(state, 32);
   lu_BitstreamRead_RecordMixingGiftData(state, &v->data);
}

void lu_BitstreamWrite_RecordMixingGift(struct lu_BitstreamState* state, const struct RecordMixingGift* v) {
   lu_BitstreamWrite_s32(state, v->checksum, 32);
   lu_BitstreamWrite_RecordMixingGiftData(state, &v->data);
}
