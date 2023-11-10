#include "lu/generated/struct-serialize//serialize_TrainerHillSave.h"

#include "global.h"

#include "lu/bitstreams.h"

void lu_BitstreamRead_TrainerHillSave(struct lu_BitstreamState* state, struct TrainerHillSave* v) {
   v->timer = lu_BitstreamRead_u32(state, 32);
   v->bestTime = lu_BitstreamRead_u32(state, 32);
   v->unk_3D6C = lu_BitstreamRead_u8(state, 8);
   v->unused = lu_BitstreamRead_u8(state, 8);
   v->receivedPrize = lu_BitstreamRead_bool(state);
   v->checkedFinalTime = lu_BitstreamRead_bool(state);
   v->spokeToOwner = lu_BitstreamRead_bool(state);
   v->hasLost = lu_BitstreamRead_bool(state);
   v->maybeECardScanDuringChallenge = lu_BitstreamRead_bool(state);
   v->field_3D6E_0f = lu_BitstreamRead_bool(state);
   v->mode = lu_BitstreamRead_u8(state, 2);
}

void lu_BitstreamWrite_TrainerHillSave(struct lu_BitstreamState* state, const struct TrainerHillSave* v) {
   lu_BitstreamWrite_u32(state, v->timer, 32);
   lu_BitstreamWrite_u32(state, v->bestTime, 32);
   lu_BitstreamWrite_u8(state, v->unk_3D6C, 8);
   lu_BitstreamWrite_u8(state, v->unused, 8);
   lu_BitstreamWrite_bool(state, v->receivedPrize);
   lu_BitstreamWrite_bool(state, v->checkedFinalTime);
   lu_BitstreamWrite_bool(state, v->spokeToOwner);
   lu_BitstreamWrite_bool(state, v->hasLost);
   lu_BitstreamWrite_bool(state, v->maybeECardScanDuringChallenge);
   lu_BitstreamWrite_bool(state, v->field_3D6E_0f);
   lu_BitstreamWrite_u8(state, v->mode, 2);
}
