#include "lu/generated/struct-serialize//serialize_Roamer.h"

#include "global.h"

#include "lu/bitstreams.h"

void lu_BitstreamRead_Roamer(struct lu_BitstreamState* state, struct Roamer* v) {
   v->ivs = lu_BitstreamRead_u32(state, 32);
   v->personality = lu_BitstreamRead_u32(state, 32);
   v->species = lu_BitstreamRead_u16(state, 11);
   v->hp = lu_BitstreamRead_u16(state, 16);
   v->level = lu_BitstreamRead_u8(state, 7);
   v->status = lu_BitstreamRead_u8(state, 8);
   v->cool = lu_BitstreamRead_u8(state, 8);
   v->beauty = lu_BitstreamRead_u8(state, 8);
   v->cute = lu_BitstreamRead_u8(state, 8);
   v->smart = lu_BitstreamRead_u8(state, 8);
   v->tough = lu_BitstreamRead_u8(state, 8);
   v->active = lu_BitstreamRead_bool(state);
}

void lu_BitstreamWrite_Roamer(struct lu_BitstreamState* state, const struct Roamer* v) {
   lu_BitstreamWrite_u32(state, v->ivs, 32);
   lu_BitstreamWrite_u32(state, v->personality, 32);
   lu_BitstreamWrite_u16(state, v->species, 11);
   lu_BitstreamWrite_u16(state, v->hp, 16);
   lu_BitstreamWrite_u8(state, v->level, 7);
   lu_BitstreamWrite_u8(state, v->status, 8);
   lu_BitstreamWrite_u8(state, v->cool, 8);
   lu_BitstreamWrite_u8(state, v->beauty, 8);
   lu_BitstreamWrite_u8(state, v->cute, 8);
   lu_BitstreamWrite_u8(state, v->smart, 8);
   lu_BitstreamWrite_u8(state, v->tough, 8);
   lu_BitstreamWrite_bool(state, v->active);
}
