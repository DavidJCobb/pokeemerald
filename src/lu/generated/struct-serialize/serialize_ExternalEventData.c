#include "lu/generated/struct-serialize//serialize_ExternalEventData.h"

#include "global.h"

#include "lu/bitstreams.h"

void lu_BitstreamRead_ExternalEventData(struct lu_BitstreamState* state, struct ExternalEventData* v) {
   {
      u16 i;
      for (i = 0; i < 7; ++i) { 
         v->unknownExternalDataFields1[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   v->unknownExternalDataFields2 = lu_BitstreamRead_u8(state, 8);
   v->currentPokeCoupons = lu_BitstreamRead_u32(state, 24);
   v->gotGoldPokeCouponTitleReward = lu_BitstreamRead_bool(state);
   v->gotSilverPokeCouponTitleReward = lu_BitstreamRead_bool(state);
   v->gotBronzePokeCouponTitleReward = lu_BitstreamRead_bool(state);
   v->receivedAgetoCelebi = lu_BitstreamRead_bool(state);
   v->unknownExternalDataFields3 = lu_BitstreamRead_u8(state, 4);
   v->totalEarnedPokeCoupons = lu_BitstreamRead_u32(state, 24);
   {
      u16 i;
      for (i = 0; i < 5; ++i) { 
         v->unknownExternalDataFields4[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
}

void lu_BitstreamWrite_ExternalEventData(struct lu_BitstreamState* state, const struct ExternalEventData* v) {
   {
      u16 i;
      for (i = 0; i < 7; ++i) { 
         lu_BitstreamWrite_u8(state, v->unknownExternalDataFields1[i], 8);
      }
   }
   lu_BitstreamWrite_u8(state, v->unknownExternalDataFields2, 8);
   lu_BitstreamWrite_u32(state, v->currentPokeCoupons, 24);
   lu_BitstreamWrite_bool(state, v->gotGoldPokeCouponTitleReward);
   lu_BitstreamWrite_bool(state, v->gotSilverPokeCouponTitleReward);
   lu_BitstreamWrite_bool(state, v->gotBronzePokeCouponTitleReward);
   lu_BitstreamWrite_bool(state, v->receivedAgetoCelebi);
   lu_BitstreamWrite_u8(state, v->unknownExternalDataFields3, 4);
   lu_BitstreamWrite_u32(state, v->totalEarnedPokeCoupons, 24);
   {
      u16 i;
      for (i = 0; i < 5; ++i) { 
         lu_BitstreamWrite_u8(state, v->unknownExternalDataFields4[i], 8);
      }
   }
}
