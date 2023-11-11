#include "lu/generated/struct-serialize//serialize_CustomGameScaleAndClamp.h"

#include "global.h"
#include "lu/custom_game_options.h" // struct definition

#include "lu/bitstreams.h"

void lu_BitstreamRead_CustomGameScaleAndClamp(struct lu_BitstreamState* state, struct CustomGameScaleAndClamp* v) {
   v->scale = lu_BitstreamRead_u16(state, 16);
   v->min = lu_BitstreamRead_u16(state, 16);
   v->max = lu_BitstreamRead_u16(state, 16);
}

void lu_BitstreamWrite_CustomGameScaleAndClamp(struct lu_BitstreamState* state, const struct CustomGameScaleAndClamp* v) {
   lu_BitstreamWrite_u16(state, v->scale, 16);
   lu_BitstreamWrite_u16(state, v->min, 16);
   lu_BitstreamWrite_u16(state, v->max, 16);
}
