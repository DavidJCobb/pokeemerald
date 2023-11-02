#include "lu/generated/struct-serialize//serialize_BerryPickingResults.h"

#include "global.h"

#include "lu/bitstreams.h"

void lu_BitstreamRead_BerryPickingResults(struct lu_BitstreamState* state, struct BerryPickingResults* v) {
   v->bestScore = lu_BitstreamRead_u32(state, 32);
   v->berriesPicked = lu_BitstreamRead_u16(state, 16);
   v->berriesPickedInRow = lu_BitstreamRead_u16(state, 16);
   v->field_8 = lu_BitstreamRead_u8(state, 8);
   v->field_9 = lu_BitstreamRead_u8(state, 8);
   v->field_A = lu_BitstreamRead_u8(state, 8);
   v->field_B = lu_BitstreamRead_u8(state, 8);
   v->field_C = lu_BitstreamRead_u8(state, 8);
   v->field_D = lu_BitstreamRead_u8(state, 8);
   v->field_E = lu_BitstreamRead_u8(state, 8);
   v->field_F = lu_BitstreamRead_u8(state, 8);
}

void lu_BitstreamWrite_BerryPickingResults(struct lu_BitstreamState* state, const struct BerryPickingResults* v) {
   lu_BitstreamWrite_u32(state, v->bestScore, 32);
   lu_BitstreamWrite_u16(state, v->berriesPicked, 16);
   lu_BitstreamWrite_u16(state, v->berriesPickedInRow, 16);
   lu_BitstreamWrite_u8(state, v->field_8, 8);
   lu_BitstreamWrite_u8(state, v->field_9, 8);
   lu_BitstreamWrite_u8(state, v->field_A, 8);
   lu_BitstreamWrite_u8(state, v->field_B, 8);
   lu_BitstreamWrite_u8(state, v->field_C, 8);
   lu_BitstreamWrite_u8(state, v->field_D, 8);
   lu_BitstreamWrite_u8(state, v->field_E, 8);
   lu_BitstreamWrite_u8(state, v->field_F, 8);
}
