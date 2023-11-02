#include "lu/generated/struct-serialize//serialize_ContestWinner.h"

#include "global.h"

#include "lu/bitstreams.h"

#include "string_util.h" // gflib; for StringLength

// check constants:
#if POKEMON_NAME_LENGTH != 10
   #error Constant `POKEMON_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if PLAYER_NAME_LENGTH != 7
   #error Constant `PLAYER_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_ContestWinner(struct lu_BitstreamState* state, struct ContestWinner* v) {
   v->personality = lu_BitstreamRead_u32(state, 32);
   v->trainerId = lu_BitstreamRead_u32(state, 32);
   v->species = lu_BitstreamRead_u16(state, 11) + 0;
   v->contestCategory = lu_BitstreamRead_u8(state, 8);
   lu_BitstreamRead_string(state, v->monName, POKEMON_NAME_LENGTH, 4);
   lu_BitstreamRead_string(state, v->trainerName, PLAYER_NAME_LENGTH, 3);
   v->contestRank = lu_BitstreamRead_u8(state, 8);
}

void lu_BitstreamWrite_ContestWinner(struct lu_BitstreamState* state, const struct ContestWinner* v) {
   lu_BitstreamWrite_u32(state, v->personality, 32);
   lu_BitstreamWrite_u32(state, v->trainerId, 32);
   lu_BitstreamWrite_u16(state, v->species - 0, 11);
   lu_BitstreamWrite_u8(state, v->contestCategory, 8);
   lu_BitstreamWrite_string(state, v->monName, POKEMON_NAME_LENGTH, 4);
   lu_BitstreamWrite_string(state, v->trainerName, PLAYER_NAME_LENGTH, 3);
   lu_BitstreamWrite_u8(state, v->contestRank, 8);
}
