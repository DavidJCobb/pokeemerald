#include "lu/generated/struct-serialize//serialize_BerryCrush.h"

#include "global.h"

#include "lu/bitstreams.h"

void lu_BitstreamRead_BerryCrush(struct lu_BitstreamState* state, struct BerryCrush* v) {
   {
      u16 i;
      for (i = 0; i < 4; ++i) { 
         v->pressingSpeeds[i] = lu_BitstreamRead_u16(state, 16);
      }
   }
   v->berryPowderAmount = lu_BitstreamRead_u32(state, 32);
   v->unk = lu_BitstreamRead_u32(state, 32);
}

void lu_BitstreamWrite_BerryCrush(struct lu_BitstreamState* state, const struct BerryCrush* v) {
   {
      u16 i;
      for (i = 0; i < 4; ++i) { 
         lu_BitstreamWrite_u16(state, v->pressingSpeeds[i], 16);
      }
   }
   lu_BitstreamWrite_u32(state, v->berryPowderAmount, 32);
   lu_BitstreamWrite_u32(state, v->unk, 32);
}
