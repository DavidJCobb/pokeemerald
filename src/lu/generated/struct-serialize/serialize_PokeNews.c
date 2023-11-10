#include "lu/generated/struct-serialize//serialize_PokeNews.h"

#include "global.h"
#include "global.tv.h" // struct definition

#include "lu/bitstreams.h"

void lu_BitstreamRead_PokeNews(struct lu_BitstreamState* state, PokeNews* v) {
   v->kind = lu_BitstreamRead_u8(state, 8);
   v->state = lu_BitstreamRead_u8(state, 8);
   v->dayCountdown = lu_BitstreamRead_u16(state, 16);
}

void lu_BitstreamWrite_PokeNews(struct lu_BitstreamState* state, const PokeNews* v) {
   lu_BitstreamWrite_u8(state, v->kind, 8);
   lu_BitstreamWrite_u8(state, v->state, 8);
   lu_BitstreamWrite_u16(state, v->dayCountdown, 16);
}
