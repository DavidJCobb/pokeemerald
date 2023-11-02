#include "lu/generated/struct-serialize//serialize_LinkBattleRecord.h"

#include "global.h"

#include "lu/bitstreams.h"

#include "string_util.h" // gflib; for StringLength

// check constants:
#if PLAYER_NAME_LENGTH != 7
   #error Constant `PLAYER_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_LinkBattleRecord(struct lu_BitstreamState* state, struct LinkBattleRecord* v) {
   lu_BitstreamRead_string(state, v->name, PLAYER_NAME_LENGTH, 3);
   v->trainerId = lu_BitstreamRead_u16(state, 16);
   v->wins = lu_BitstreamRead_u16(state, 16);
   v->losses = lu_BitstreamRead_u16(state, 16);
   v->draws = lu_BitstreamRead_u16(state, 16);
}

void lu_BitstreamWrite_LinkBattleRecord(struct lu_BitstreamState* state, const struct LinkBattleRecord* v) {
   lu_BitstreamWrite_string(state, v->name, PLAYER_NAME_LENGTH, 3);
   lu_BitstreamWrite_u16(state, v->trainerId, 16);
   lu_BitstreamWrite_u16(state, v->wins, 16);
   lu_BitstreamWrite_u16(state, v->losses, 16);
   lu_BitstreamWrite_u16(state, v->draws, 16);
}
