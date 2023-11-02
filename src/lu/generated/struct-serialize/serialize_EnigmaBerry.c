#include "lu/generated/struct-serialize//serialize_EnigmaBerry.h"

#include "global.h"
#include "global.berry.h" // struct definition

#include "lu/bitstreams.h"

// dependencies
#include "lu/generated/struct-serialize//serialize_Berry2.h"

// check constants:
#if BERRY_ITEM_EFFECT_COUNT != 18
   #error Constant `BERRY_ITEM_EFFECT_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_EnigmaBerry(struct lu_BitstreamState* state, struct EnigmaBerry* v) {
   lu_BitstreamRead_Berry2(state, &v->berry);
   {
      u16 i;
      for (i = 0; i < BERRY_ITEM_EFFECT_COUNT; ++i) { 
         v->itemEffect[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   v->holdEffect = lu_BitstreamRead_u8(state, 8);
   v->holdEffectParam = lu_BitstreamRead_u8(state, 8);
   v->checksum = lu_BitstreamRead_u32(state, 32);
}

void lu_BitstreamWrite_EnigmaBerry(struct lu_BitstreamState* state, const struct EnigmaBerry* v) {
   lu_BitstreamWrite_Berry2(state, &v->berry);
   {
      u16 i;
      for (i = 0; i < BERRY_ITEM_EFFECT_COUNT; ++i) { 
         lu_BitstreamWrite_u8(state, v->itemEffect[i], 8);
      }
   }
   lu_BitstreamWrite_u8(state, v->holdEffect, 8);
   lu_BitstreamWrite_u8(state, v->holdEffectParam, 8);
   lu_BitstreamWrite_u32(state, v->checksum, 32);
}
