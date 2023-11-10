#include "lu/generated/struct-serialize//serialize_Time.h"

#include "global.h"

#include "lu/bitstreams.h"

void lu_BitstreamRead_Time(struct lu_BitstreamState* state, struct Time* v) {
   v->days = lu_BitstreamRead_s16(state, 16);
   v->hours = lu_BitstreamRead_u8(state, 6) + -24;
   v->minutes = lu_BitstreamRead_u8(state, 7) + -60;
   v->seconds = lu_BitstreamRead_u8(state, 7) + -60;
}

void lu_BitstreamWrite_Time(struct lu_BitstreamState* state, const struct Time* v) {
   lu_BitstreamWrite_s16(state, v->days, 16);
   lu_BitstreamWrite_u8(state, v->hours - -24, 6);
   lu_BitstreamWrite_u8(state, v->minutes - -60, 7);
   lu_BitstreamWrite_u8(state, v->seconds - -60, 7);
}
