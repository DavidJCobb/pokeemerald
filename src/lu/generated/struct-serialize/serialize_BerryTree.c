#include "lu/generated/struct-serialize//serialize_BerryTree.h"

#include "global.h"
#include "global.berry.h" // struct definition

#include "lu/bitstreams.h"

void lu_BitstreamRead_BerryTree(struct lu_BitstreamState* state, struct BerryTree* v) {
   v->berry = lu_BitstreamRead_u8(state, 8);
   v->stage = lu_BitstreamRead_u8(state, 7);
   v->stopGrowth = lu_BitstreamRead_bool(state);
   v->minutesUntilNextStage = lu_BitstreamRead_u16(state, 16);
   v->berryYield = lu_BitstreamRead_u8(state, 8);
   v->regrowthCount = lu_BitstreamRead_u8(state, 4);
   v->watered1 = lu_BitstreamRead_bool(state);
   v->watered2 = lu_BitstreamRead_bool(state);
   v->watered3 = lu_BitstreamRead_bool(state);
   v->watered4 = lu_BitstreamRead_bool(state);
}

void lu_BitstreamWrite_BerryTree(struct lu_BitstreamState* state, const struct BerryTree* v) {
   lu_BitstreamWrite_u8(state, v->berry, 8);
   lu_BitstreamWrite_u8(state, v->stage, 7);
   lu_BitstreamWrite_bool(state, v->stopGrowth);
   lu_BitstreamWrite_u16(state, v->minutesUntilNextStage, 16);
   lu_BitstreamWrite_u8(state, v->berryYield, 8);
   lu_BitstreamWrite_u8(state, v->regrowthCount, 4);
   lu_BitstreamWrite_bool(state, v->watered1);
   lu_BitstreamWrite_bool(state, v->watered2);
   lu_BitstreamWrite_bool(state, v->watered3);
   lu_BitstreamWrite_bool(state, v->watered4);
}
