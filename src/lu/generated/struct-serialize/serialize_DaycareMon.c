#include "lu/generated/struct-serialize//serialize_DaycareMon.h"

#include "global.h"

#include "lu/bitstreams.h"

// dependencies
#include "lu/generated/struct-serialize//serialize_BoxPokemon.h"
#include "lu/generated/struct-serialize//serialize_DaycareMail.h"

void lu_BitstreamRead_DaycareMon(struct lu_BitstreamState* state, struct DaycareMon* v) {
   lu_BitstreamRead_BoxPokemon(state, &v->mon);
   lu_BitstreamRead_DaycareMail(state, &v->mail);
   v->steps = lu_BitstreamRead_u32(state, 32);
}

void lu_BitstreamWrite_DaycareMon(struct lu_BitstreamState* state, const struct DaycareMon* v) {
   lu_BitstreamWrite_BoxPokemon(state, &v->mon);
   lu_BitstreamWrite_DaycareMail(state, &v->mail);
   lu_BitstreamWrite_u32(state, v->steps, 32);
}
