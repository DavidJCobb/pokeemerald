#include "lu/generated/struct-serialize//serialize_Pokemon.h"

#include "global.h"
#include "pokemon.h" // struct definition

#include "lu/bitstreams.h"

// dependencies
#include "lu/generated/struct-serialize//serialize_BoxPokemon.h"

void lu_BitstreamRead_Pokemon(struct lu_BitstreamState* state, struct Pokemon* v) {
   lu_BitstreamRead_BoxPokemon(state, &v->box);
   v->status = lu_BitstreamRead_u32(state, 32);
   v->level = lu_BitstreamRead_u8(state, 7) + 0;
   v->mail = lu_BitstreamRead_u8(state, 8);
   v->hp = lu_BitstreamRead_u16(state, 16);
   v->maxHP = lu_BitstreamRead_u16(state, 16);
   v->attack = lu_BitstreamRead_u16(state, 16);
   v->defense = lu_BitstreamRead_u16(state, 16);
   v->speed = lu_BitstreamRead_u16(state, 16);
   v->spAttack = lu_BitstreamRead_u16(state, 16);
   v->spDefense = lu_BitstreamRead_u16(state, 16);
}

void lu_BitstreamWrite_Pokemon(struct lu_BitstreamState* state, const struct Pokemon* v) {
   lu_BitstreamWrite_BoxPokemon(state, &v->box);
   lu_BitstreamWrite_u32(state, v->status, 32);
   lu_BitstreamWrite_u8(state, v->level, 7);
   lu_BitstreamWrite_u8(state, v->mail, 8);
   lu_BitstreamWrite_u16(state, v->hp, 16);
   lu_BitstreamWrite_u16(state, v->maxHP, 16);
   lu_BitstreamWrite_u16(state, v->attack, 16);
   lu_BitstreamWrite_u16(state, v->defense, 16);
   lu_BitstreamWrite_u16(state, v->speed, 16);
   lu_BitstreamWrite_u16(state, v->spAttack, 16);
   lu_BitstreamWrite_u16(state, v->spDefense, 16);
}
