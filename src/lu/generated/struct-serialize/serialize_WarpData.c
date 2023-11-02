#include "lu/generated/struct-serialize//serialize_WarpData.h"

#include "global.h"

#include "lu/bitstreams.h"

void lu_BitstreamRead_WarpData(struct lu_BitstreamState* state, struct WarpData* v) {
   v->mapGroup = lu_BitstreamRead_s8(state, 8);
   v->mapNum = lu_BitstreamRead_s8(state, 8);
   v->warpId = lu_BitstreamRead_s8(state, 8);
   v->x = lu_BitstreamRead_s16(state, 16);
   v->y = lu_BitstreamRead_s16(state, 16);
}

void lu_BitstreamWrite_WarpData(struct lu_BitstreamState* state, const struct WarpData* v) {
   lu_BitstreamWrite_s8(state, v->mapGroup, 8);
   lu_BitstreamWrite_s8(state, v->mapNum, 8);
   lu_BitstreamWrite_s8(state, v->warpId, 8);
   lu_BitstreamWrite_s16(state, v->x, 16);
   lu_BitstreamWrite_s16(state, v->y, 16);
}
