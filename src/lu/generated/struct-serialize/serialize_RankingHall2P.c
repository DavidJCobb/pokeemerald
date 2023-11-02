#include "lu/generated/struct-serialize//serialize_RankingHall2P.h"

#include "global.h"

#include "lu/bitstreams.h"

#include "string_util.h" // gflib; for StringLength

// check constants:
#if TRAINER_ID_LENGTH != 4
   #error Constant `TRAINER_ID_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if PLAYER_NAME_LENGTH != 7
   #error Constant `PLAYER_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_RankingHall2P(struct lu_BitstreamState* state, struct RankingHall2P* v) {
   {
      u16 i;
      for (i = 0; i < TRAINER_ID_LENGTH; ++i) { 
         v->id1[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < TRAINER_ID_LENGTH; ++i) { 
         v->id2[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   v->winStreak = lu_BitstreamRead_u16(state, 16);
   lu_BitstreamRead_string(state, v->name1, PLAYER_NAME_LENGTH, 3);
   lu_BitstreamRead_string(state, v->name2, PLAYER_NAME_LENGTH, 3);
   v->language = lu_BitstreamRead_u8(state, 3) + 0;
}

void lu_BitstreamWrite_RankingHall2P(struct lu_BitstreamState* state, const struct RankingHall2P* v) {
   {
      u16 i;
      for (i = 0; i < TRAINER_ID_LENGTH; ++i) { 
         lu_BitstreamWrite_u8(state, v->id1[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < TRAINER_ID_LENGTH; ++i) { 
         lu_BitstreamWrite_u8(state, v->id2[i], 8);
      }
   }
   lu_BitstreamWrite_u16(state, v->winStreak, 16);
   lu_BitstreamWrite_string(state, v->name1, PLAYER_NAME_LENGTH, 3);
   lu_BitstreamWrite_string(state, v->name2, PLAYER_NAME_LENGTH, 3);
   lu_BitstreamWrite_u8(state, v->language - 0, 3);
}
