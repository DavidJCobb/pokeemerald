#include "lu/generated/struct-serialize/serialize_ApprenticeMon.h"

#include "global.h"

#include "lu/bitstreams.h"

// check constants:
#if MAX_MON_MOVES != 4
   #error Constant `MAX_MON_MOVES` has been changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_ApprenticeMon(struct lu_BitstreamState* state, struct ApprenticeMon* v) {
   u8 i;
   v->species = lu_BitstreamRead_u16(state, 11);
   for (i = 0; i < MAX_MON_MOVES; ++i) {
      v->moves[i] = lu_BitstreamRead_u16(state, 16);
   }
   v->item = lu_BitstreamRead_u16(state, 9);
}

void lu_BitstreamWrite_ApprenticeMon(struct lu_BitstreamState* state, const struct ApprenticeMon* v) {
   u8 i;
   lu_BitstreamWrite_u16(state, v->species, 11);
   for (i = 0; i < MAX_MON_MOVES; ++i) {
      lu_BitstreamWrite_u16(state, v->moves[i], 16);
   }
   lu_BitstreamWrite_u16(state, v->item, 9);
}