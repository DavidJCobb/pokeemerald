#include "lu/generated/struct-serialize//serialize_WonderCardMetadata.h"

#include "global.h"

#include "lu/bitstreams.h"

// check constants:
#if MAX_STAMP_CARD_STAMPS != 7
   #error Constant `MAX_STAMP_CARD_STAMPS` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_WonderCardMetadata(struct lu_BitstreamState* state, struct WonderCardMetadata* v) {
   v->battlesWon = lu_BitstreamRead_u16(state, 16);
   v->battlesLost = lu_BitstreamRead_u16(state, 16);
   v->numTrades = lu_BitstreamRead_u16(state, 16);
   v->iconSpecies = lu_BitstreamRead_u16(state, 11) + 0;
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < MAX_STAMP_CARD_STAMPS; ++j) { 
            v->stampData[i][j] = lu_BitstreamRead_u16(state, 16);
         }
      }
   }
}

void lu_BitstreamWrite_WonderCardMetadata(struct lu_BitstreamState* state, const struct WonderCardMetadata* v) {
   lu_BitstreamWrite_u16(state, v->battlesWon, 16);
   lu_BitstreamWrite_u16(state, v->battlesLost, 16);
   lu_BitstreamWrite_u16(state, v->numTrades, 16);
   lu_BitstreamWrite_u16(state, v->iconSpecies, 11);
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < MAX_STAMP_CARD_STAMPS; ++j) { 
            lu_BitstreamWrite_u16(state, v->stampData[i][j], 16);
         }
      }
   }
}
