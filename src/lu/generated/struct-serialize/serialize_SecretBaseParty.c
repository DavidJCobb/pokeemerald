#include "lu/generated/struct-serialize//serialize_SecretBaseParty.h"

#include "global.h"

#include "lu/bitstreams.h"

// check constants:
#if PARTY_SIZE != 6
   #error Constant `PARTY_SIZE` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_SecretBaseParty(struct lu_BitstreamState* state, struct SecretBaseParty* v) {
   u8 i;
   for (i = 0; i < PARTY_SIZE; ++i) {
      v->personality[i] = lu_BitstreamRead_u32(state, 32);
   }
   for (i = 0; i < PARTY_SIZE * MAX_MON_MOVES; ++i) {
      v->moves[i] = lu_BitstreamRead_u16(state, 16);
   }
   for (i = 0; i < PARTY_SIZE; ++i) {
      v->species[i] = lu_BitstreamRead_u16(state, 11);
   }
   for (i = 0; i < PARTY_SIZE; ++i) {
      v->heldItems[i] = lu_BitstreamRead_u16(state, 9);
   }
   for (i = 0; i < PARTY_SIZE; ++i) {
      v->levels[i] = lu_BitstreamRead_u8(state, 7);
   }
   for (i = 0; i < PARTY_SIZE; ++i) {
      v->EVs[i] = lu_BitstreamRead_u8(state, 8);
   }
}

void lu_BitstreamWrite_SecretBaseParty(struct lu_BitstreamState* state, const struct SecretBaseParty* v) {
   u8 i;
   for (i = 0; i < PARTY_SIZE; ++i) {
      lu_BitstreamWrite_u32(state, v->personality[i], 32);
   }
   for (i = 0; i < PARTY_SIZE * MAX_MON_MOVES; ++i) {
      lu_BitstreamWrite_u16(state, v->moves[i], 16);
   }
   for (i = 0; i < PARTY_SIZE; ++i) {
      lu_BitstreamWrite_u16(state, v->species[i], 11);
   }
   for (i = 0; i < PARTY_SIZE; ++i) {
      lu_BitstreamWrite_u16(state, v->heldItems[i], 9);
   }
   for (i = 0; i < PARTY_SIZE; ++i) {
      lu_BitstreamWrite_u8(state, v->levels[i], 7);
   }
   for (i = 0; i < PARTY_SIZE; ++i) {
      lu_BitstreamWrite_u8(state, v->EVs[i], 8);
   }
}
