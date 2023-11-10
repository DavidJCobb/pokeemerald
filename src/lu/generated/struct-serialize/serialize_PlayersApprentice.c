#include "lu/generated/struct-serialize//serialize_PlayersApprentice.h"

#include "global.h"

#include "lu/bitstreams.h"

// dependencies
#include "lu/generated/struct-serialize//serialize_ApprenticeQuestion.h"

// check constants:
#if MULTI_PARTY_SIZE != 3
   #error Constant `MULTI_PARTY_SIZE` changed in C, but XML not updated or codegen not re-run!
#endif
#if APPRENTICE_MAX_QUESTIONS != 9
   #error Constant `APPRENTICE_MAX_QUESTIONS` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_PlayersApprentice(struct lu_BitstreamState* state, struct PlayersApprentice* v) {
   u8 i;
   v->id = lu_BitstreamRead_u8(state, 8);
   v->lvlMode = lu_BitstreamRead_u8(state, 2);
   v->questionsAnswered = lu_BitstreamRead_u8(state, 4);
   v->leadMonId = lu_BitstreamRead_u8(state, 2);
   v->party = lu_BitstreamRead_u8(state, 3);
   v->saveId = lu_BitstreamRead_u8(state, 2);
   for (i = 0; i < MULTI_PARTY_SIZE; ++i) {
      v->speciesIds[i] = lu_BitstreamRead_u8(state, 8);
   }
   for (i = 0; i < APPRENTICE_MAX_QUESTIONS; ++i) {
      lu_BitstreamRead_ApprenticeQuestion(state, &v->questions[i]);
   }
}

void lu_BitstreamWrite_PlayersApprentice(struct lu_BitstreamState* state, const struct PlayersApprentice* v) {
   u8 i;
   lu_BitstreamWrite_u8(state, v->id, 8);
   lu_BitstreamWrite_u8(state, v->lvlMode, 2);
   lu_BitstreamWrite_u8(state, v->questionsAnswered, 4);
   lu_BitstreamWrite_u8(state, v->leadMonId, 2);
   lu_BitstreamWrite_u8(state, v->party, 3);
   lu_BitstreamWrite_u8(state, v->saveId, 2);
   for (i = 0; i < MULTI_PARTY_SIZE; ++i) {
      lu_BitstreamWrite_u8(state, v->speciesIds[i], 8);
   }
   for (i = 0; i < APPRENTICE_MAX_QUESTIONS; ++i) {
      lu_BitstreamWrite_ApprenticeQuestion(state, &v->questions[i]);
   }
}
