#include "lu/generated/struct-serialize/serialize_CustomGameSavestate.h"

#include "global.h"
#include "lu/custom_game_options.h" // struct definition

#include "lu/bitstreams.h"

void lu_BitstreamRead_CustomGameSavestate(struct lu_BitstreamState* state, struct CustomGameSavestate* v) {
   v->has_ever_had_poke_balls = lu_BitstreamRead_bool(state);
}

void lu_BitstreamWrite_CustomGameSavestate(struct lu_BitstreamState* state, const struct CustomGameSavestate* v) {
   lu_BitstreamWrite_bool(state, v->has_ever_had_poke_balls);
}
