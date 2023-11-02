#include "lu/generated/struct-serialize//serialize_DewfordTrend.h"

#include "global.h"

#include "lu/bitstreams.h"

void lu_BitstreamRead_DewfordTrend(struct lu_BitstreamState* state, struct DewfordTrend* v) {
   v->trendiness = lu_BitstreamRead_u8(state, 7);
   v->maxTrendiness = lu_BitstreamRead_u8(state, 7);
   v->gainingTrendiness = lu_BitstreamRead_bool(state);
   v->rand = lu_BitstreamRead_u16(state, 16);
   {
      u16 i;
      for (i = 0; i < 2; ++i) { 
         v->words[i] = lu_BitstreamRead_u16(state, 16);
      }
   }
}

void lu_BitstreamWrite_DewfordTrend(struct lu_BitstreamState* state, const struct DewfordTrend* v) {
   lu_BitstreamWrite_u8(state, v->trendiness, 7);
   lu_BitstreamWrite_u8(state, v->maxTrendiness, 7);
   lu_BitstreamWrite_bool(state, v->gainingTrendiness);
   lu_BitstreamWrite_u16(state, v->rand, 16);
   {
      u16 i;
      for (i = 0; i < 2; ++i) { 
         lu_BitstreamWrite_u16(state, v->words[i], 16);
      }
   }
}
