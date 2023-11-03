#include "lu/generated/struct-serialize//serialize_DomeMonData.h"

#include "global.h"

#include "lu/bitstreams.h"

// check constants:
#if MAX_MON_MOVES != 4
   #error Constant `MAX_MON_MOVES` changed in C, but XML not updated or codegen not re-run!
#endif
#if NUM_STATS != 6
   #error Constant `NUM_STATS` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_DomeMonData(struct lu_BitstreamState* state, struct DomeMonData* v) {
   {
      u16 i;
      for (i = 0; i < MAX_MON_MOVES; ++i) { 
         v->moves[i] = lu_BitstreamRead_u16(state, 16) + 0;
      }
   }
   {
      u16 i;
      for (i = 0; i < NUM_STATS; ++i) { 
         v->evs[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   v->nature = lu_BitstreamRead_u8(state, 8);
}

void lu_BitstreamWrite_DomeMonData(struct lu_BitstreamState* state, const struct DomeMonData* v) {
   {
      u16 i;
      for (i = 0; i < MAX_MON_MOVES; ++i) { 
         lu_BitstreamWrite_u16(state, v->moves[i], 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < NUM_STATS; ++i) { 
         lu_BitstreamWrite_u8(state, v->evs[i], 8);
      }
   }
   lu_BitstreamWrite_u8(state, v->nature, 8);
}
