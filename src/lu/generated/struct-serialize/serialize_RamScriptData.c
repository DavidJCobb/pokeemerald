#include "lu/generated/struct-serialize//serialize_RamScriptData.h"

#include "global.h"

#include "lu/bitstreams.h"

void lu_BitstreamRead_RamScriptData(struct lu_BitstreamState* state, struct RamScriptData* v) {
   v->magic = lu_BitstreamRead_u8(state, 8);
   v->mapGroup = lu_BitstreamRead_u8(state, 8);
   v->mapNum = lu_BitstreamRead_u8(state, 8);
   v->objectId = lu_BitstreamRead_u8(state, 8);
   {
      u16 i;
      for (i = 0; i < 995; ++i) { 
         v->script[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
}

void lu_BitstreamWrite_RamScriptData(struct lu_BitstreamState* state, const struct RamScriptData* v) {
   lu_BitstreamWrite_u8(state, v->magic, 8);
   lu_BitstreamWrite_u8(state, v->mapGroup, 8);
   lu_BitstreamWrite_u8(state, v->mapNum, 8);
   lu_BitstreamWrite_u8(state, v->objectId, 8);
   {
      u16 i;
      for (i = 0; i < 995; ++i) { 
         lu_BitstreamWrite_u8(state, v->script[i], 8);
      }
   }
}
