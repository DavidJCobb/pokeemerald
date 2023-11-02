#include "lu/generated/struct-serialize//serialize_SecretBaseParty.h"

#include "global.h"

#include "lu/bitstreams.h"

// check constants:
#if PARTY_SIZE != 6
   #error Constant `PARTY_SIZE` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_SecretBaseParty(struct lu_BitstreamState* state, struct SecretBaseParty* v) {
   {
      u16 i;
      for (i = 0; i < PARTY_SIZE; ++i) { 
         v->personality[i] = lu_BitstreamRead_u32(state, 32);
      }
   }
   {
      u16 i;
      for (i = 0; i < PARTY_SIZE * MAX_MON_MOVES; ++i) { 
         v->moves[i] = lu_BitstreamRead_u16(state, 16) + 0;
      }
   }
   {
      u16 i;
      for (i = 0; i < PARTY_SIZE; ++i) { 
         v->species[i] = lu_BitstreamRead_u16(state, 11) + 0;
      }
   }
   {
      u16 i;
      for (i = 0; i < PARTY_SIZE; ++i) { 
         v->heldItems[i] = lu_BitstreamRead_u16(state, 9);
      }
   }
   {
      u16 i;
      for (i = 0; i < PARTY_SIZE; ++i) { 
         v->levels[i] = lu_BitstreamRead_u8(state, 7) + 0;
      }
   }
   {
      u16 i;
      for (i = 0; i < PARTY_SIZE; ++i) { 
         v->EVs[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
}

void lu_BitstreamWrite_SecretBaseParty(struct lu_BitstreamState* state, const struct SecretBaseParty* v) {
   {
      u16 i;
      for (i = 0; i < PARTY_SIZE; ++i) { 
         lu_BitstreamWrite_u32(state, v->personality[i], 32);
      }
   }
   {
      u16 i;
      for (i = 0; i < PARTY_SIZE * MAX_MON_MOVES; ++i) { 
         lu_BitstreamWrite_u16(state, v->moves[i] - 0, 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < PARTY_SIZE; ++i) { 
         lu_BitstreamWrite_u16(state, v->species[i] - 0, 11);
      }
   }
   {
      u16 i;
      for (i = 0; i < PARTY_SIZE; ++i) { 
         lu_BitstreamWrite_u16(state, v->heldItems[i], 9);
      }
   }
   {
      u16 i;
      for (i = 0; i < PARTY_SIZE; ++i) { 
         lu_BitstreamWrite_u8(state, v->levels[i] - 0, 7);
      }
   }
   {
      u16 i;
      for (i = 0; i < PARTY_SIZE; ++i) { 
         lu_BitstreamWrite_u8(state, v->EVs[i], 8);
      }
   }
}
