#include "lu/generated/struct-serialize//serialize_WonderNewsMetadata.h"

#include "global.h"

#include "lu/bitstreams.h"

void lu_BitstreamRead_WonderNewsMetadata(struct lu_BitstreamState* state, struct WonderNewsMetadata* v) {
   v->newsType = lu_BitstreamRead_u8(state, 2);
   v->sentRewardCounter = lu_BitstreamRead_u8(state, 3);
   v->rewardCounter = lu_BitstreamRead_u8(state, 3);
   v->berry = lu_BitstreamRead_u8(state, 8);
}

void lu_BitstreamWrite_WonderNewsMetadata(struct lu_BitstreamState* state, const struct WonderNewsMetadata* v) {
   lu_BitstreamWrite_u8(state, v->newsType, 2);
   lu_BitstreamWrite_u8(state, v->sentRewardCounter, 3);
   lu_BitstreamWrite_u8(state, v->rewardCounter, 3);
   lu_BitstreamWrite_u8(state, v->berry, 8);
}
