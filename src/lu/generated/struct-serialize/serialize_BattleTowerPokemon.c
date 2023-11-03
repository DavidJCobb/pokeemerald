#include "lu/generated/struct-serialize//serialize_BattleTowerPokemon.h"

#include "global.h"

#include "lu/bitstreams.h"

#include "string_util.h" // gflib; for StringLength

// check constants:
#if MAX_MON_MOVES != 4
   #error Constant `MAX_MON_MOVES` changed in C, but XML not updated or codegen not re-run!
#endif
#if POKEMON_NAME_LENGTH != 10
   #error Constant `POKEMON_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_BattleTowerPokemon(struct lu_BitstreamState* state, struct BattleTowerPokemon* v) {
   v->species = lu_BitstreamRead_u16(state, 11) + 0;
   v->heldItem = lu_BitstreamRead_u16(state, 9);
   {
      u16 i;
      for (i = 0; i < MAX_MON_MOVES; ++i) { 
         v->moves[i] = lu_BitstreamRead_u16(state, 16) + 0;
      }
   }
   v->level = lu_BitstreamRead_u8(state, 7) + 0;
   v->ppBonuses = lu_BitstreamRead_u8(state, 8);
   v->hpEV = lu_BitstreamRead_u8(state, 8);
   v->attackEV = lu_BitstreamRead_u8(state, 8);
   v->defenseEV = lu_BitstreamRead_u8(state, 8);
   v->speedEV = lu_BitstreamRead_u8(state, 8);
   v->spAttackEV = lu_BitstreamRead_u8(state, 8);
   v->spDefenseEV = lu_BitstreamRead_u8(state, 8);
   v->otId = lu_BitstreamRead_u32(state, 32);
   v->hpIV = lu_BitstreamRead_u8(state, 5);
   v->attackIV = lu_BitstreamRead_u8(state, 5);
   v->defenseIV = lu_BitstreamRead_u8(state, 5);
   v->speedIV = lu_BitstreamRead_u8(state, 5);
   v->spAttackIV = lu_BitstreamRead_u8(state, 5);
   v->spDefenseIV = lu_BitstreamRead_u8(state, 5);
   v->abilityNum = lu_BitstreamRead_bool(state);
   v->personality = lu_BitstreamRead_u32(state, 32);
   lu_BitstreamRead_string(state, v->nickname, POKEMON_NAME_LENGTH, 4);
   v->friendship = lu_BitstreamRead_u8(state, 8);
}

void lu_BitstreamWrite_BattleTowerPokemon(struct lu_BitstreamState* state, const struct BattleTowerPokemon* v) {
   lu_BitstreamWrite_u16(state, v->species, 11);
   lu_BitstreamWrite_u16(state, v->heldItem, 9);
   {
      u16 i;
      for (i = 0; i < MAX_MON_MOVES; ++i) { 
         lu_BitstreamWrite_u16(state, v->moves[i], 16);
      }
   }
   lu_BitstreamWrite_u8(state, v->level, 7);
   lu_BitstreamWrite_u8(state, v->ppBonuses, 8);
   lu_BitstreamWrite_u8(state, v->hpEV, 8);
   lu_BitstreamWrite_u8(state, v->attackEV, 8);
   lu_BitstreamWrite_u8(state, v->defenseEV, 8);
   lu_BitstreamWrite_u8(state, v->speedEV, 8);
   lu_BitstreamWrite_u8(state, v->spAttackEV, 8);
   lu_BitstreamWrite_u8(state, v->spDefenseEV, 8);
   lu_BitstreamWrite_u32(state, v->otId, 32);
   lu_BitstreamWrite_u8(state, v->hpIV, 5);
   lu_BitstreamWrite_u8(state, v->attackIV, 5);
   lu_BitstreamWrite_u8(state, v->defenseIV, 5);
   lu_BitstreamWrite_u8(state, v->speedIV, 5);
   lu_BitstreamWrite_u8(state, v->spAttackIV, 5);
   lu_BitstreamWrite_u8(state, v->spDefenseIV, 5);
   lu_BitstreamWrite_bool(state, v->abilityNum);
   lu_BitstreamWrite_u32(state, v->personality, 32);
   lu_BitstreamWrite_string(state, v->nickname, POKEMON_NAME_LENGTH, 4);
   lu_BitstreamWrite_u8(state, v->friendship, 8);
}
