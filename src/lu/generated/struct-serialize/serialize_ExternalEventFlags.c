#include "lu/generated/struct-serialize//serialize_ExternalEventFlags.h"

#include "global.h"

#include "lu/bitstreams.h"

void lu_BitstreamRead_ExternalEventFlags(struct lu_BitstreamState* state, struct ExternalEventFlags* v) {
   v->usedBoxRS = lu_BitstreamRead_bool(state);
   v->boxRSEggsUnlocked = lu_BitstreamRead_u8(state, 2);
   v->unknownFlag1 = lu_BitstreamRead_u8(state, 8);
   v->receivedGCNJirachi = lu_BitstreamRead_u8(state, 8);
   v->unknownFlag3 = lu_BitstreamRead_u8(state, 8);
   v->unknownFlag4 = lu_BitstreamRead_u8(state, 8);
   v->unknownFlag5 = lu_BitstreamRead_u8(state, 8);
   v->unknownFlag6 = lu_BitstreamRead_u8(state, 8);
   v->unknownFlag7 = lu_BitstreamRead_u8(state, 8);
   v->unknownFlag8 = lu_BitstreamRead_u8(state, 8);
   v->unknownFlag9 = lu_BitstreamRead_u8(state, 8);
   v->unknownFlag10 = lu_BitstreamRead_u8(state, 8);
   v->unknownFlag11 = lu_BitstreamRead_u8(state, 8);
   v->unknownFlag12 = lu_BitstreamRead_u8(state, 8);
   v->unknownFlag13 = lu_BitstreamRead_u8(state, 8);
   v->unknownFlag14 = lu_BitstreamRead_u8(state, 8);
   v->unknownFlag15 = lu_BitstreamRead_u8(state, 8);
   v->unknownFlag16 = lu_BitstreamRead_u8(state, 8);
   v->unknownFlag17 = lu_BitstreamRead_u8(state, 8);
   v->unknownFlag18 = lu_BitstreamRead_u8(state, 8);
   v->unknownFlag19 = lu_BitstreamRead_u8(state, 8);
   v->unknownFlag20 = lu_BitstreamRead_u8(state, 8);
}

void lu_BitstreamWrite_ExternalEventFlags(struct lu_BitstreamState* state, const struct ExternalEventFlags* v) {
   lu_BitstreamWrite_bool(state, v->usedBoxRS);
   lu_BitstreamWrite_u8(state, v->boxRSEggsUnlocked, 2);
   lu_BitstreamWrite_u8(state, v->unknownFlag1, 8);
   lu_BitstreamWrite_u8(state, v->receivedGCNJirachi, 8);
   lu_BitstreamWrite_u8(state, v->unknownFlag3, 8);
   lu_BitstreamWrite_u8(state, v->unknownFlag4, 8);
   lu_BitstreamWrite_u8(state, v->unknownFlag5, 8);
   lu_BitstreamWrite_u8(state, v->unknownFlag6, 8);
   lu_BitstreamWrite_u8(state, v->unknownFlag7, 8);
   lu_BitstreamWrite_u8(state, v->unknownFlag8, 8);
   lu_BitstreamWrite_u8(state, v->unknownFlag9, 8);
   lu_BitstreamWrite_u8(state, v->unknownFlag10, 8);
   lu_BitstreamWrite_u8(state, v->unknownFlag11, 8);
   lu_BitstreamWrite_u8(state, v->unknownFlag12, 8);
   lu_BitstreamWrite_u8(state, v->unknownFlag13, 8);
   lu_BitstreamWrite_u8(state, v->unknownFlag14, 8);
   lu_BitstreamWrite_u8(state, v->unknownFlag15, 8);
   lu_BitstreamWrite_u8(state, v->unknownFlag16, 8);
   lu_BitstreamWrite_u8(state, v->unknownFlag17, 8);
   lu_BitstreamWrite_u8(state, v->unknownFlag18, 8);
   lu_BitstreamWrite_u8(state, v->unknownFlag19, 8);
   lu_BitstreamWrite_u8(state, v->unknownFlag20, 8);
}
