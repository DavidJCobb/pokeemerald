#include "lu/generated/struct-serialize//serialize_BoxPokemon.h"

#include "global.h"
#include "pokemon.h" // struct definition

#include "lu/bitstreams.h"

#include "string_util.h" // gflib; for StringLength

// check constants:
#if POKEMON_NAME_LENGTH != 10
   #error Constant `POKEMON_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if PLAYER_NAME_LENGTH != 7
   #error Constant `PLAYER_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_BoxPokemon(struct lu_BitstreamState* state, struct BoxPokemon* v) {
   u8 i;
   v->personality = lu_BitstreamRead_u32(state, 32);
   v->otId = lu_BitstreamRead_u32(state, 32);
   lu_BitstreamRead_string_optional_terminator(state, v->nickname, POKEMON_NAME_LENGTH);
   v->language = lu_BitstreamRead_u8(state, 3);
   v->isBadEgg = lu_BitstreamRead_bool(state);
   v->hasSpecies = lu_BitstreamRead_bool(state);
   v->isEgg = lu_BitstreamRead_bool(state);
   v->unused = lu_BitstreamRead_u8(state, 5);
   lu_BitstreamRead_string_optional_terminator(state, v->otName, PLAYER_NAME_LENGTH);
   v->markings = lu_BitstreamRead_u8(state, 8);
   v->checksum = lu_BitstreamRead_u16(state, 16);
   v->unknown = lu_BitstreamRead_u16(state, 16);
   for (i = 0; i < (NUM_SUBSTRUCT_BYTES * 4) / 4; ++i) {
      v->secure.raw[i] = lu_BitstreamRead_u32(state, 32);
   }
}

void lu_BitstreamWrite_BoxPokemon(struct lu_BitstreamState* state, const struct BoxPokemon* v) {
   u8 i;
   lu_BitstreamWrite_u32(state, v->personality, 32);
   lu_BitstreamWrite_u32(state, v->otId, 32);
   lu_BitstreamWrite_string_optional_terminator(state, v->nickname, POKEMON_NAME_LENGTH);
   lu_BitstreamWrite_u8(state, v->language, 3);
   lu_BitstreamWrite_bool(state, v->isBadEgg);
   lu_BitstreamWrite_bool(state, v->hasSpecies);
   lu_BitstreamWrite_bool(state, v->isEgg);
   lu_BitstreamWrite_u8(state, v->unused, 5);
   lu_BitstreamWrite_string_optional_terminator(state, v->otName, PLAYER_NAME_LENGTH);
   lu_BitstreamWrite_u8(state, v->markings, 8);
   lu_BitstreamWrite_u16(state, v->checksum, 16);
   lu_BitstreamWrite_u16(state, v->unknown, 16);
   for (i = 0; i < (NUM_SUBSTRUCT_BYTES * 4) / 4; ++i) {
      lu_BitstreamWrite_u32(state, v->secure.raw[i], 32);
   }
}
