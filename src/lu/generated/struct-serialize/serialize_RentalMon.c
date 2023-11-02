#include "lu/generated/struct-serialize//serialize_RentalMon.h"

#include "global.h"

#include "lu/bitstreams.h"

void lu_BitstreamRead_RentalMon(struct lu_BitstreamState* state, struct RentalMon* v) {
   v->monId = lu_BitstreamRead_u16(state, 11) + 0;
   v->personality = lu_BitstreamRead_u32(state, 32);
   v->ivs = lu_BitstreamRead_u8(state, 8);
   v->abilityNum = lu_BitstreamRead_u8(state, 8);
}

void lu_BitstreamWrite_RentalMon(struct lu_BitstreamState* state, const struct RentalMon* v) {
   lu_BitstreamWrite_u16(state, v->monId - 0, 11);
   lu_BitstreamWrite_u32(state, v->personality, 32);
   lu_BitstreamWrite_u8(state, v->ivs, 8);
   lu_BitstreamWrite_u8(state, v->abilityNum, 8);
}
