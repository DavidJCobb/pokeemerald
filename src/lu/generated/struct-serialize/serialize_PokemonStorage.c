#include "lu/generated/struct-serialize//serialize_PokemonStorage.h"

#include "global.h"
#include "pokemon_storage_system.h" // struct definition

#include "lu/bitstreams.h"

// dependencies
#include "lu/generated/struct-serialize//serialize_BoxPokemon.h"

#include "string_util.h" // gflib; for StringLength

// check constants:
#if TOTAL_BOXES_COUNT != 14
   #error Constant `TOTAL_BOXES_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if IN_BOX_COUNT != 30
   #error Constant `IN_BOX_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if BOX_NAME_LENGTH != 8
   #error Constant `BOX_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if TOTAL_BOXES_COUNT != 14
   #error Constant `TOTAL_BOXES_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_PokemonStorage(struct lu_BitstreamState* state, struct PokemonStorage* v) {
   u8 i, j;
   v->currentBox = lu_BitstreamRead_u8(state, 4);
   for (i = 0; i < TOTAL_BOXES_COUNT; ++i) {
      for (j = 0; j < IN_BOX_COUNT; ++j) {
         lu_BitstreamRead_BoxPokemon(state, &v->boxes[i][j]);
      }
   }
   for (i = 0; i < TOTAL_BOXES_COUNT; ++i) {
      lu_BitstreamRead_string(state, v->boxNames[i], BOX_NAME_LENGTH);
   }
   for (i = 0; i < TOTAL_BOXES_COUNT; ++i) {
      v->boxWallpapers[i] = lu_BitstreamRead_u8(state, 8);
   }
}

void lu_BitstreamWrite_PokemonStorage(struct lu_BitstreamState* state, const struct PokemonStorage* v) {
   u8 i, j;
   lu_BitstreamWrite_u8(state, v->currentBox, 4);
   for (i = 0; i < TOTAL_BOXES_COUNT; ++i) {
      for (j = 0; j < IN_BOX_COUNT; ++j) {
         lu_BitstreamWrite_BoxPokemon(state, &v->boxes[i][j]);
      }
   }
   for (i = 0; i < TOTAL_BOXES_COUNT; ++i) {
      lu_BitstreamWrite_string(state, v->boxNames[i], BOX_NAME_LENGTH);
   }
   for (i = 0; i < TOTAL_BOXES_COUNT; ++i) {
      lu_BitstreamWrite_u8(state, v->boxWallpapers[i], 8);
   }
}
