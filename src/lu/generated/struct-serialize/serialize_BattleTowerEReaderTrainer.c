#include "lu/generated/struct-serialize//serialize_BattleTowerEReaderTrainer.h"

#include "global.h"

#include "lu/bitstreams.h"

// dependencies
#include "lu/generated/struct-serialize//serialize_BattleTowerPokemon.h"

#include "string_util.h" // gflib; for StringLength

// check constants:
#if PLAYER_NAME_LENGTH != 7
   #error Constant `PLAYER_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if TRAINER_ID_LENGTH != 4
   #error Constant `TRAINER_ID_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if EASY_CHAT_BATTLE_WORDS_COUNT != 6
   #error Constant `EASY_CHAT_BATTLE_WORDS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if FRONTIER_PARTY_SIZE != 3
   #error Constant `FRONTIER_PARTY_SIZE` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_BattleTowerEReaderTrainer(struct lu_BitstreamState* state, struct BattleTowerEReaderTrainer* v) {
   u8 i;
   v->unk0 = lu_BitstreamRead_u8(state, 8);
   v->facilityClass = lu_BitstreamRead_u8(state, 8);
   v->winStreak = lu_BitstreamRead_u16(state, 16);
   lu_BitstreamRead_string(state, v->name, PLAYER_NAME_LENGTH);
   for (i = 0; i < TRAINER_ID_LENGTH; ++i) {
      v->trainerId[i] = lu_BitstreamRead_u8(state, 8);
   }
   for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) {
      v->greeting[i] = lu_BitstreamRead_u16(state, 16);
   }
   for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) {
      v->farewellPlayerLost[i] = lu_BitstreamRead_u16(state, 16);
   }
   for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) {
      v->farewellPlayerWon[i] = lu_BitstreamRead_u16(state, 16);
   }
   for (i = 0; i < FRONTIER_PARTY_SIZE; ++i) {
      lu_BitstreamRead_BattleTowerPokemon(state, &v->party[i]);
   }
   v->checksum = lu_BitstreamRead_u32(state, 32);
}

void lu_BitstreamWrite_BattleTowerEReaderTrainer(struct lu_BitstreamState* state, const struct BattleTowerEReaderTrainer* v) {
   u8 i;
   lu_BitstreamWrite_u8(state, v->unk0, 8);
   lu_BitstreamWrite_u8(state, v->facilityClass, 8);
   lu_BitstreamWrite_u16(state, v->winStreak, 16);
   lu_BitstreamWrite_string(state, v->name, PLAYER_NAME_LENGTH);
   for (i = 0; i < TRAINER_ID_LENGTH; ++i) {
      lu_BitstreamWrite_u8(state, v->trainerId[i], 8);
   }
   for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) {
      lu_BitstreamWrite_u16(state, v->greeting[i], 16);
   }
   for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) {
      lu_BitstreamWrite_u16(state, v->farewellPlayerLost[i], 16);
   }
   for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) {
      lu_BitstreamWrite_u16(state, v->farewellPlayerWon[i], 16);
   }
   for (i = 0; i < FRONTIER_PARTY_SIZE; ++i) {
      lu_BitstreamWrite_BattleTowerPokemon(state, &v->party[i]);
   }
   lu_BitstreamWrite_u32(state, v->checksum, 32);
}
