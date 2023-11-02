#include "lu/generated/struct-serialize//serialize_ObjectEventTemplate.h"

#include "global.h"
#include "global.fieldmap.h" // struct definition

#include "lu/bitstreams.h"

void lu_BitstreamRead_ObjectEventTemplate(struct lu_BitstreamState* state, struct ObjectEventTemplate* v) {
   v->localId = lu_BitstreamRead_u8(state, 8);
   v->graphicsId = lu_BitstreamRead_u8(state, 8);
   v->kind = lu_BitstreamRead_u8(state, 8);
   v->x = lu_BitstreamRead_s16(state, 16);
   v->y = lu_BitstreamRead_s16(state, 16);
   v->elevation = lu_BitstreamRead_u8(state, 8);
   v->movementType = lu_BitstreamRead_u8(state, 8);
   v->movementRangeX = lu_BitstreamRead_u8(state, 4);
   v->movementRangeY = lu_BitstreamRead_u8(state, 4);
   v->trainerType = lu_BitstreamRead_u16(state, 16);
   v->trainerRange_berryTreeId = lu_BitstreamRead_u16(state, 16);
   v->script = (u8*) lu_BitstreamRead_u32(state, 32);
   v->flagId = lu_BitstreamRead_u16(state, 16);
}

void lu_BitstreamWrite_ObjectEventTemplate(struct lu_BitstreamState* state, const struct ObjectEventTemplate* v) {
   lu_BitstreamWrite_u8(state, v->localId, 8);
   lu_BitstreamWrite_u8(state, v->graphicsId, 8);
   lu_BitstreamWrite_u8(state, v->kind, 8);
   lu_BitstreamWrite_s16(state, v->x, 16);
   lu_BitstreamWrite_s16(state, v->y, 16);
   lu_BitstreamWrite_u8(state, v->elevation, 8);
   lu_BitstreamWrite_u8(state, v->movementType, 8);
   lu_BitstreamWrite_u8(state, v->movementRangeX, 4);
   lu_BitstreamWrite_u8(state, v->movementRangeY, 4);
   lu_BitstreamWrite_u16(state, v->trainerType, 16);
   lu_BitstreamWrite_u16(state, v->trainerRange_berryTreeId, 16);
   lu_BitstreamWrite_u32(state, (u32)v->script, 32);
   lu_BitstreamWrite_u16(state, v->flagId, 16);
}
