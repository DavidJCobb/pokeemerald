#include "lu/generated/struct-serialize//serialize_ApprenticeQuestion.h"

#include "global.h"

#include "lu/bitstreams.h"

void lu_BitstreamRead_ApprenticeQuestion(struct lu_BitstreamState* state, struct ApprenticeQuestion* v) {
   v->questionId = lu_BitstreamRead_u8(state, 2);
   v->monId = lu_BitstreamRead_u8(state, 2);
   v->moveSlot = lu_BitstreamRead_u8(state, 2);
   v->suggestedChange = lu_BitstreamRead_u8(state, 2);
   v->data = lu_BitstreamRead_u16(state, 16);
}

void lu_BitstreamWrite_ApprenticeQuestion(struct lu_BitstreamState* state, const struct ApprenticeQuestion* v) {
   lu_BitstreamWrite_u8(state, v->questionId, 2);
   lu_BitstreamWrite_u8(state, v->monId, 2);
   lu_BitstreamWrite_u8(state, v->moveSlot, 2);
   lu_BitstreamWrite_u8(state, v->suggestedChange, 2);
   lu_BitstreamWrite_u16(state, v->data, 16);
}
