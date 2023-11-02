#include "lu/generated/struct-serialize//serialize_GabbyAndTyData.h"

#include "global.h"
#include "global.tv.h" // struct definition

#include "lu/bitstreams.h"

void lu_BitstreamRead_GabbyAndTyData(struct lu_BitstreamState* state, struct GabbyAndTyData* v) {
   v->mon1 = lu_BitstreamRead_u16(state, 11) + 0;
   v->mon2 = lu_BitstreamRead_u16(state, 11) + 0;
   v->lastMove = lu_BitstreamRead_u16(state, 16) + 0;
   {
      u16 i;
      for (i = 0; i < 1; ++i) { 
         v->quote[i] = lu_BitstreamRead_u16(state, 16);
      }
   }
   v->mapnum = lu_BitstreamRead_u8(state, 8);
   v->battleNum = lu_BitstreamRead_u8(state, 8);
   v->battleTookMoreThanOneTurn = lu_BitstreamRead_bool(state);
   v->playerLostAMon = lu_BitstreamRead_bool(state);
   v->playerUsedHealingItem = lu_BitstreamRead_bool(state);
   v->playerThrewABall = lu_BitstreamRead_bool(state);
   v->onAir = lu_BitstreamRead_bool(state);
   v->valA_5 = lu_BitstreamRead_u8(state, 3);
   v->battleTookMoreThanOneTurn2 = lu_BitstreamRead_bool(state);
   v->playerLostAMon2 = lu_BitstreamRead_bool(state);
   v->playerUsedHealingItem2 = lu_BitstreamRead_bool(state);
   v->playerThrewABall2 = lu_BitstreamRead_bool(state);
   v->valB_4 = lu_BitstreamRead_u8(state, 4);
}

void lu_BitstreamWrite_GabbyAndTyData(struct lu_BitstreamState* state, const struct GabbyAndTyData* v) {
   lu_BitstreamWrite_u16(state, v->mon1 - 0, 11);
   lu_BitstreamWrite_u16(state, v->mon2 - 0, 11);
   lu_BitstreamWrite_u16(state, v->lastMove - 0, 16);
   {
      u16 i;
      for (i = 0; i < 1; ++i) { 
         lu_BitstreamWrite_u16(state, v->quote[i], 16);
      }
   }
   lu_BitstreamWrite_u8(state, v->mapnum, 8);
   lu_BitstreamWrite_u8(state, v->battleNum, 8);
   lu_BitstreamWrite_bool(state, v->battleTookMoreThanOneTurn);
   lu_BitstreamWrite_bool(state, v->playerLostAMon);
   lu_BitstreamWrite_bool(state, v->playerUsedHealingItem);
   lu_BitstreamWrite_bool(state, v->playerThrewABall);
   lu_BitstreamWrite_bool(state, v->onAir);
   lu_BitstreamWrite_u8(state, v->valA_5, 3);
   lu_BitstreamWrite_bool(state, v->battleTookMoreThanOneTurn2);
   lu_BitstreamWrite_bool(state, v->playerLostAMon2);
   lu_BitstreamWrite_bool(state, v->playerUsedHealingItem2);
   lu_BitstreamWrite_bool(state, v->playerThrewABall2);
   lu_BitstreamWrite_u8(state, v->valB_4, 4);
}
