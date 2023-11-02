#include "lu/generated/struct-serialize//serialize_Berry2.h"

#include "global.h"
#include "global.berry.h" // struct definition

#include "lu/bitstreams.h"

#include "string_util.h" // gflib; for StringLength

// check constants:
#if BERRY_NAME_LENGTH != 6
   #error Constant `BERRY_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_Berry2(struct lu_BitstreamState* state, struct Berry2* v) {
   lu_BitstreamRead_string(state, v->name, BERRY_NAME_LENGTH, 3);
   v->firmness = lu_BitstreamRead_u8(state, 8);
   v->size = lu_BitstreamRead_u16(state, 16);
   v->maxYield = lu_BitstreamRead_u8(state, 8);
   v->minYield = lu_BitstreamRead_u8(state, 8);
   v->description1 = (u8*) lu_BitstreamRead_u32(state, 32);
   v->description2 = (u8*) lu_BitstreamRead_u32(state, 32);
   v->stageDuration = lu_BitstreamRead_u8(state, 8);
   v->spicy = lu_BitstreamRead_u8(state, 8);
   v->dry = lu_BitstreamRead_u8(state, 8);
   v->sweet = lu_BitstreamRead_u8(state, 8);
   v->bitter = lu_BitstreamRead_u8(state, 8);
   v->sour = lu_BitstreamRead_u8(state, 8);
   v->smoothness = lu_BitstreamRead_u8(state, 8);
}

void lu_BitstreamWrite_Berry2(struct lu_BitstreamState* state, const struct Berry2* v) {
   lu_BitstreamWrite_string(state, v->name, BERRY_NAME_LENGTH, 3);
   lu_BitstreamWrite_u8(state, v->firmness, 8);
   lu_BitstreamWrite_u16(state, v->size, 16);
   lu_BitstreamWrite_u8(state, v->maxYield, 8);
   lu_BitstreamWrite_u8(state, v->minYield, 8);
   lu_BitstreamWrite_u32(state, (u32)v->description1, 32);
   lu_BitstreamWrite_u32(state, (u32)v->description2, 32);
   lu_BitstreamWrite_u8(state, v->stageDuration, 8);
   lu_BitstreamWrite_u8(state, v->spicy, 8);
   lu_BitstreamWrite_u8(state, v->dry, 8);
   lu_BitstreamWrite_u8(state, v->sweet, 8);
   lu_BitstreamWrite_u8(state, v->bitter, 8);
   lu_BitstreamWrite_u8(state, v->sour, 8);
   lu_BitstreamWrite_u8(state, v->smoothness, 8);
}
