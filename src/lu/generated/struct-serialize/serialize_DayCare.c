#include "lu/generated/struct-serialize//serialize_DayCare.h"

#include "global.h"

#include "lu/bitstreams.h"

// dependencies
#include "lu/generated/struct-serialize//serialize_DaycareMon.h"

// check constants:
#if DAYCARE_MON_COUNT != 2
   #error Constant `DAYCARE_MON_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_DayCare(struct lu_BitstreamState* state, struct DayCare* v) {
   u8 i;
   for (i = 0; i < DAYCARE_MON_COUNT; ++i) {
      lu_BitstreamRead_DaycareMon(state, &v->mons[i]);
   }
   v->offspringPersonality = lu_BitstreamRead_u32(state, 32);
   v->stepCounter = lu_BitstreamRead_u8(state, 8);
}

void lu_BitstreamWrite_DayCare(struct lu_BitstreamState* state, const struct DayCare* v) {
   u8 i;
   for (i = 0; i < DAYCARE_MON_COUNT; ++i) {
      lu_BitstreamWrite_DaycareMon(state, &v->mons[i]);
   }
   lu_BitstreamWrite_u32(state, v->offspringPersonality, 32);
   lu_BitstreamWrite_u8(state, v->stepCounter, 8);
}
