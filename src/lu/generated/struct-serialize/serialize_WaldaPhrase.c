#include "lu/generated/struct-serialize//serialize_WaldaPhrase.h"

#include "global.h"

#include "lu/bitstreams.h"

void lu_BitstreamRead_WaldaPhrase(struct lu_BitstreamState* state, struct WaldaPhrase* v) {
   {
      u16 i;
      for (i = 0; i < 2; ++i) { 
         v->colors[i] = lu_BitstreamRead_u16(state, 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < 16; ++i) { 
         v->text[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   v->iconId = lu_BitstreamRead_u8(state, 8);
   v->patternId = lu_BitstreamRead_u8(state, 8);
   v->patternUnlocked = lu_BitstreamRead_bool(state);
}

void lu_BitstreamWrite_WaldaPhrase(struct lu_BitstreamState* state, const struct WaldaPhrase* v) {
   {
      u16 i;
      for (i = 0; i < 2; ++i) { 
         lu_BitstreamWrite_u16(state, v->colors[i], 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < 16; ++i) { 
         lu_BitstreamWrite_u8(state, v->text[i], 8);
      }
   }
   lu_BitstreamWrite_u8(state, v->iconId, 8);
   lu_BitstreamWrite_u8(state, v->patternId, 8);
   lu_BitstreamWrite_bool(state, v->patternUnlocked);
}
