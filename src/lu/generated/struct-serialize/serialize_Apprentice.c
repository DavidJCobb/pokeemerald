#include "lu/generated/struct-serialize//serialize_Apprentice.h"

#include "global.h"

#include "lu/bitstreams.h"

// dependencies
#include "lu/generated/struct-serialize//serialize_ApprenticeMon.h"

#include "string_util.h" // gflib; for StringLength

// check constants:
#if MULTI_PARTY_SIZE != 3
   #error Constant `MULTI_PARTY_SIZE` changed in C, but XML not updated or codegen not re-run!
#endif
#if EASY_CHAT_BATTLE_WORDS_COUNT != 6
   #error Constant `EASY_CHAT_BATTLE_WORDS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if TRAINER_ID_LENGTH != 4
   #error Constant `TRAINER_ID_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if PLAYER_NAME_LENGTH != 7
   #error Constant `PLAYER_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_Apprentice(struct lu_BitstreamState* state, struct Apprentice* v) {
   v->id = lu_BitstreamRead_u8(state, 5);
   v->lvlMode = lu_BitstreamRead_u8(state, 2);
   v->numQuestions = lu_BitstreamRead_u8(state, 8);
   v->number = lu_BitstreamRead_u8(state, 8);
   {
      u16 i;
      for (i = 0; i < MULTI_PARTY_SIZE; ++i) { 
         lu_BitstreamRead_ApprenticeMon(state, &v->party[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) { 
         v->speechWon[i] = lu_BitstreamRead_u16(state, 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < TRAINER_ID_LENGTH; ++i) { 
         v->playerId[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   lu_BitstreamRead_string_optional_terminator(state, v->playerName, PLAYER_NAME_LENGTH);
   v->language = lu_BitstreamRead_u8(state, 3) + 0;
   v->checksum = lu_BitstreamRead_u32(state, 32);
}

void lu_BitstreamWrite_Apprentice(struct lu_BitstreamState* state, const struct Apprentice* v) {
   lu_BitstreamWrite_u8(state, v->id, 5);
   lu_BitstreamWrite_u8(state, v->lvlMode, 2);
   lu_BitstreamWrite_u8(state, v->numQuestions, 8);
   lu_BitstreamWrite_u8(state, v->number, 8);
   {
      u16 i;
      for (i = 0; i < MULTI_PARTY_SIZE; ++i) { 
         lu_BitstreamWrite_ApprenticeMon(state, &v->party[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) { 
         lu_BitstreamWrite_u16(state, v->speechWon[i], 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < TRAINER_ID_LENGTH; ++i) { 
         lu_BitstreamWrite_u8(state, v->playerId[i], 8);
      }
   }
   lu_BitstreamWrite_string_optional_terminator(state, v->playerName, PLAYER_NAME_LENGTH);
   lu_BitstreamWrite_u8(state, v->language - 0, 3);
   lu_BitstreamWrite_u32(state, v->checksum, 32);
}
