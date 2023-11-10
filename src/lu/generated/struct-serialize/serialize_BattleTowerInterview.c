#include "lu/generated/struct-serialize//serialize_BattleTowerInterview.h"

#include "global.h"

#include "lu/bitstreams.h"

#include "string_util.h" // gflib; for StringLength

// check constants:
#if PLAYER_NAME_LENGTH != 7
   #error Constant `PLAYER_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if POKEMON_NAME_LENGTH != 10
   #error Constant `POKEMON_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_BattleTowerInterview(struct lu_BitstreamState* state, struct BattleTowerInterview* v) {
   v->playerSpecies = lu_BitstreamRead_u16(state, 11);
   v->opponentSpecies = lu_BitstreamRead_u16(state, 11);
   lu_BitstreamRead_string(state, v->opponentName, PLAYER_NAME_LENGTH);
   lu_BitstreamRead_string(state, v->opponentMonNickname, POKEMON_NAME_LENGTH);
   v->opponentLanguage = lu_BitstreamRead_u8(state, 3);
}

void lu_BitstreamWrite_BattleTowerInterview(struct lu_BitstreamState* state, const struct BattleTowerInterview* v) {
   lu_BitstreamWrite_u16(state, v->playerSpecies, 11);
   lu_BitstreamWrite_u16(state, v->opponentSpecies, 11);
   lu_BitstreamWrite_string(state, v->opponentName, PLAYER_NAME_LENGTH);
   lu_BitstreamWrite_string(state, v->opponentMonNickname, POKEMON_NAME_LENGTH);
   lu_BitstreamWrite_u8(state, v->opponentLanguage, 3);
}
