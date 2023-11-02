#include "lu/generated/struct-serialize//serialize_Pokedex.h"

#include "global.h"

#include "lu/bitstreams.h"

// check constants:
#if NUM_DEX_FLAG_BYTES != 52
   #error Constant `NUM_DEX_FLAG_BYTES` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_Pokedex(struct lu_BitstreamState* state, struct Pokedex* v) {
   v->order = lu_BitstreamRead_u8(state, 8);
   v->mode = lu_BitstreamRead_u8(state, 8);
   v->nationalMagic = lu_BitstreamRead_u8(state, 8);
   v->unknown2 = lu_BitstreamRead_u8(state, 8);
   v->unownPersonality = lu_BitstreamRead_u32(state, 32);
   v->spindaPersonality = lu_BitstreamRead_u32(state, 32);
   v->unknown3 = lu_BitstreamRead_u32(state, 32);
   {
      u16 i;
      for (i = 0; i < NUM_DEX_FLAG_BYTES; ++i) { 
         v->owned[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < NUM_DEX_FLAG_BYTES; ++i) { 
         v->seen[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
}

void lu_BitstreamWrite_Pokedex(struct lu_BitstreamState* state, const struct Pokedex* v) {
   lu_BitstreamWrite_u8(state, v->order, 8);
   lu_BitstreamWrite_u8(state, v->mode, 8);
   lu_BitstreamWrite_u8(state, v->nationalMagic, 8);
   lu_BitstreamWrite_u8(state, v->unknown2, 8);
   lu_BitstreamWrite_u32(state, v->unownPersonality, 32);
   lu_BitstreamWrite_u32(state, v->spindaPersonality, 32);
   lu_BitstreamWrite_u32(state, v->unknown3, 32);
   {
      u16 i;
      for (i = 0; i < NUM_DEX_FLAG_BYTES; ++i) { 
         lu_BitstreamWrite_u8(state, v->owned[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < NUM_DEX_FLAG_BYTES; ++i) { 
         lu_BitstreamWrite_u8(state, v->seen[i], 8);
      }
   }
}
