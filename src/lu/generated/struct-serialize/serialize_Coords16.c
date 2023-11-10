#include "lu/generated/struct-serialize//serialize_Coords16.h"

#include "global.h"

#include "lu/bitstreams.h"

void lu_BitstreamRead_Coords16(struct lu_BitstreamState* state, struct Coords16* v) {
   v->x = lu_BitstreamRead_s16(state, 16);
   v->y = lu_BitstreamRead_s16(state, 16);
}

void lu_BitstreamWrite_Coords16(struct lu_BitstreamState* state, const struct Coords16* v) {
   lu_BitstreamWrite_s16(state, v->x, 16);
   lu_BitstreamWrite_s16(state, v->y, 16);
}
