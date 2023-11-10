#include "lu/generated/struct-serialize//serialize_Pokeblock.h"

#include "global.h"

#include "lu/bitstreams.h"

void lu_BitstreamRead_Pokeblock(struct lu_BitstreamState* state, struct Pokeblock* v) {
   v->color = lu_BitstreamRead_u8(state, 8);
   v->spicy = lu_BitstreamRead_u8(state, 8);
   v->dry = lu_BitstreamRead_u8(state, 8);
   v->sweet = lu_BitstreamRead_u8(state, 8);
   v->bitter = lu_BitstreamRead_u8(state, 8);
   v->sour = lu_BitstreamRead_u8(state, 8);
   v->feel = lu_BitstreamRead_u8(state, 8);
}

void lu_BitstreamWrite_Pokeblock(struct lu_BitstreamState* state, const struct Pokeblock* v) {
   lu_BitstreamWrite_u8(state, v->color, 8);
   lu_BitstreamWrite_u8(state, v->spicy, 8);
   lu_BitstreamWrite_u8(state, v->dry, 8);
   lu_BitstreamWrite_u8(state, v->sweet, 8);
   lu_BitstreamWrite_u8(state, v->bitter, 8);
   lu_BitstreamWrite_u8(state, v->sour, 8);
   lu_BitstreamWrite_u8(state, v->feel, 8);
}
