#include "lu/generated/struct-serialize//serialize_TrainerNameRecord.h"

#include "global.h"

#include "lu/bitstreams.h"

#include "string_util.h" // gflib; for StringLength

// check constants:
#if PLAYER_NAME_LENGTH != 7
   #error Constant `PLAYER_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_TrainerNameRecord(struct lu_BitstreamState* state, struct TrainerNameRecord* v) {
   v->trainerId = lu_BitstreamRead_u32(state, 32);
   lu_BitstreamRead_string(state, v->trainerName, PLAYER_NAME_LENGTH, 3);
}

void lu_BitstreamWrite_TrainerNameRecord(struct lu_BitstreamState* state, const struct TrainerNameRecord* v) {
   lu_BitstreamWrite_u32(state, v->trainerId, 32);
   lu_BitstreamWrite_string(state, v->trainerName, PLAYER_NAME_LENGTH, 3);
}
