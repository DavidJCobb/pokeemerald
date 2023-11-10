#include "lu/generated/struct-serialize//serialize_PokemonJumpRecords.h"

#include "global.h"

#include "lu/bitstreams.h"

void lu_BitstreamRead_PokemonJumpRecords(struct lu_BitstreamState* state, struct PokemonJumpRecords* v) {
   v->jumpsInRow = lu_BitstreamRead_u16(state, 16);
   v->unused1 = lu_BitstreamRead_u16(state, 16);
   v->excellentsInRow = lu_BitstreamRead_u16(state, 16);
   v->gamesWithMaxPlayers = lu_BitstreamRead_u16(state, 16);
   v->unused2 = lu_BitstreamRead_u16(state, 16);
   v->bestJumpScore = lu_BitstreamRead_u32(state, 32);
}

void lu_BitstreamWrite_PokemonJumpRecords(struct lu_BitstreamState* state, const struct PokemonJumpRecords* v) {
   lu_BitstreamWrite_u16(state, v->jumpsInRow, 16);
   lu_BitstreamWrite_u16(state, v->unused1, 16);
   lu_BitstreamWrite_u16(state, v->excellentsInRow, 16);
   lu_BitstreamWrite_u16(state, v->gamesWithMaxPlayers, 16);
   lu_BitstreamWrite_u16(state, v->unused2, 16);
   lu_BitstreamWrite_u32(state, v->bestJumpScore, 32);
}
