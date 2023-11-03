#include "lu/generated/struct-serialize//serialize_Mail.h"

#include "global.h"

#include "lu/bitstreams.h"

#include "string_util.h" // gflib; for StringLength

// check constants:
#if MAIL_WORDS_COUNT != 9
   #error Constant `MAIL_WORDS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if PLAYER_NAME_LENGTH != 7
   #error Constant `PLAYER_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if TRAINER_ID_LENGTH != 4
   #error Constant `TRAINER_ID_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_Mail(struct lu_BitstreamState* state, struct Mail* v) {
   {
      u16 i;
      for (i = 0; i < MAIL_WORDS_COUNT; ++i) { 
         v->words[i] = lu_BitstreamRead_u16(state, 16);
      }
   }
   lu_BitstreamRead_string(state, v->playerName, PLAYER_NAME_LENGTH, 3);
   {
      u16 i;
      for (i = 0; i < TRAINER_ID_LENGTH; ++i) { 
         v->trainerId[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   v->species = lu_BitstreamRead_u16(state, 11) + 0;
   v->itemId = lu_BitstreamRead_u16(state, 9);
}

void lu_BitstreamWrite_Mail(struct lu_BitstreamState* state, const struct Mail* v) {
   {
      u16 i;
      for (i = 0; i < MAIL_WORDS_COUNT; ++i) { 
         lu_BitstreamWrite_u16(state, v->words[i], 16);
      }
   }
   lu_BitstreamWrite_string(state, v->playerName, PLAYER_NAME_LENGTH, 3);
   {
      u16 i;
      for (i = 0; i < TRAINER_ID_LENGTH; ++i) { 
         lu_BitstreamWrite_u8(state, v->trainerId[i], 8);
      }
   }
   lu_BitstreamWrite_u16(state, v->species, 11);
   lu_BitstreamWrite_u16(state, v->itemId, 9);
}
