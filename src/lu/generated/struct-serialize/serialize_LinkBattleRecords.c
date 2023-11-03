#include "lu/generated/struct-serialize//serialize_LinkBattleRecords.h"

#include "global.h"

#include "lu/bitstreams.h"

// dependencies
#include "lu/generated/struct-serialize//serialize_LinkBattleRecord.h"

// check constants:
#if LINK_B_RECORDS_COUNT != 5
   #error Constant `LINK_B_RECORDS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_LinkBattleRecords(struct lu_BitstreamState* state, struct LinkBattleRecords* v) {
   {
      u16 i;
      for (i = 0; i < LINK_B_RECORDS_COUNT; ++i) { 
         lu_BitstreamRead_LinkBattleRecord(state, &v->entries[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < LINK_B_RECORDS_COUNT; ++i) { 
         v->languages[i] = lu_BitstreamRead_u8(state, 3) + 0;
      }
   }
}

void lu_BitstreamWrite_LinkBattleRecords(struct lu_BitstreamState* state, const struct LinkBattleRecords* v) {
   {
      u16 i;
      for (i = 0; i < LINK_B_RECORDS_COUNT; ++i) { 
         lu_BitstreamWrite_LinkBattleRecord(state, &v->entries[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < LINK_B_RECORDS_COUNT; ++i) { 
         lu_BitstreamWrite_u8(state, v->languages[i], 3);
      }
   }
}
