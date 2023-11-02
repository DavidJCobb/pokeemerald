#include "lu/generated/struct-serialize//serialize_DaycareMail.h"

#include "global.h"

#include "lu/bitstreams.h"

// dependencies
#include "lu/generated/struct-serialize//serialize_Mail.h"

#include "string_util.h" // gflib; for StringLength

// check constants:
#if PLAYER_NAME_LENGTH != 7
   #error Constant `PLAYER_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if POKEMON_NAME_LENGTH != 10
   #error Constant `POKEMON_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_DaycareMail(struct lu_BitstreamState* state, struct DaycareMail* v) {
   lu_BitstreamRead_Mail(state, &v->message);
   lu_BitstreamRead_string(state, v->otName, PLAYER_NAME_LENGTH, 3);
   lu_BitstreamRead_string(state, v->monName, POKEMON_NAME_LENGTH, 4);
   v->gameLanguage = lu_BitstreamRead_u8(state, 3) + 0;
   v->monLanguage = lu_BitstreamRead_u8(state, 3) + 0;
}

void lu_BitstreamWrite_DaycareMail(struct lu_BitstreamState* state, const struct DaycareMail* v) {
   lu_BitstreamWrite_Mail(state, &v->message);
   lu_BitstreamWrite_string(state, v->otName, PLAYER_NAME_LENGTH, 3);
   lu_BitstreamWrite_string(state, v->monName, POKEMON_NAME_LENGTH, 4);
   lu_BitstreamWrite_u8(state, v->gameLanguage - 0, 3);
   lu_BitstreamWrite_u8(state, v->monLanguage - 0, 3);
}
