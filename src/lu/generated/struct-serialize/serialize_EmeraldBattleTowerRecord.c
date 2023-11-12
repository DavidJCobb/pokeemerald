#include "lu/generated/struct-serialize/serialize_EmeraldBattleTowerRecord.h"

#include "global.h"

#include "lu/bitstreams.h"

// dependencies
#include "lu/generated/struct-serialize/serialize_BattleTowerPokemon.h"

#include "string_util.h" // gflib; for StringLength

// check constants:
#if PLAYER_NAME_LENGTH != 7
   #error Constant `PLAYER_NAME_LENGTH` has been changed in C, but XML not updated or codegen not re-run!
#endif
#if TRAINER_ID_LENGTH != 4
   #error Constant `TRAINER_ID_LENGTH` has been changed in C, but XML not updated or codegen not re-run!
#endif
#if EASY_CHAT_BATTLE_WORDS_COUNT != 6
   #error Constant `EASY_CHAT_BATTLE_WORDS_COUNT` has been changed in C, but XML not updated or codegen not re-run!
#endif
#if MAX_FRONTIER_PARTY_SIZE != 4
   #error Constant `MAX_FRONTIER_PARTY_SIZE` has been changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_EmeraldBattleTowerRecord(struct lu_BitstreamState* state, struct EmeraldBattleTowerRecord* v) {
   u8 i;
   v->lvlMode = lu_BitstreamRead_bool(state);
   v->facilityClass = lu_BitstreamRead_u8(state, 5);
   v->winStreak = lu_BitstreamRead_u16(state, 16);
   lu_BitstreamRead_string(state, v->name, PLAYER_NAME_LENGTH);
   for (i = 0; i < TRAINER_ID_LENGTH; ++i) {
      v->trainerId[i] = lu_BitstreamRead_u8(state, 8);
   }
   for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) {
      v->greeting[i] = lu_BitstreamRead_u16(state, 16);
   }
   for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) {
      v->speechWon[i] = lu_BitstreamRead_u16(state, 16);
   }
   for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) {
      v->speechLost[i] = lu_BitstreamRead_u16(state, 16);
   }
   for (i = 0; i < MAX_FRONTIER_PARTY_SIZE; ++i) {
      lu_BitstreamRead_BattleTowerPokemon(state, &v->party[i]);
   }
   v->language = lu_BitstreamRead_u8(state, 3);
   v->checksum = lu_BitstreamRead_u32(state, 32);
}

void lu_BitstreamWrite_EmeraldBattleTowerRecord(struct lu_BitstreamState* state, const struct EmeraldBattleTowerRecord* v) {
   u8 i;
   lu_BitstreamWrite_bool(state, v->lvlMode);
   lu_BitstreamWrite_u8(state, v->facilityClass, 5);
   lu_BitstreamWrite_u16(state, v->winStreak, 16);
   lu_BitstreamWrite_string(state, v->name, PLAYER_NAME_LENGTH);
   for (i = 0; i < TRAINER_ID_LENGTH; ++i) {
      lu_BitstreamWrite_u8(state, v->trainerId[i], 8);
   }
   for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) {
      lu_BitstreamWrite_u16(state, v->greeting[i], 16);
   }
   for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) {
      lu_BitstreamWrite_u16(state, v->speechWon[i], 16);
   }
   for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) {
      lu_BitstreamWrite_u16(state, v->speechLost[i], 16);
   }
   for (i = 0; i < MAX_FRONTIER_PARTY_SIZE; ++i) {
      lu_BitstreamWrite_BattleTowerPokemon(state, &v->party[i]);
   }
   lu_BitstreamWrite_u8(state, v->language, 3);
   lu_BitstreamWrite_u32(state, v->checksum, 32);
}
