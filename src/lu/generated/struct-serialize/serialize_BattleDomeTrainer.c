#include "lu/generated/struct-serialize//serialize_BattleDomeTrainer.h"

#include "global.h"

#include "lu/bitstreams.h"

void lu_BitstreamRead_BattleDomeTrainer(struct lu_BitstreamState* state, struct BattleDomeTrainer* v) {
   v->trainerId = lu_BitstreamRead_u16(state, 10);
   v->isEliminated = lu_BitstreamRead_bool(state);
   v->eliminatedAt = lu_BitstreamRead_u8(state, 2);
   v->forfeited = lu_BitstreamRead_u8(state, 3);
}

void lu_BitstreamWrite_BattleDomeTrainer(struct lu_BitstreamState* state, const struct BattleDomeTrainer* v) {
   lu_BitstreamWrite_u16(state, v->trainerId, 10);
   lu_BitstreamWrite_bool(state, v->isEliminated);
   lu_BitstreamWrite_u8(state, v->eliminatedAt, 2);
   lu_BitstreamWrite_u8(state, v->forfeited, 3);
}
