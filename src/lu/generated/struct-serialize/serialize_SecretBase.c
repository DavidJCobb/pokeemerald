#include "lu/generated/struct-serialize//serialize_SecretBase.h"

#include "global.h"

#include "lu/bitstreams.h"

// dependencies
#include "lu/generated/struct-serialize//serialize_SecretBaseParty.h"

#include "string_util.h" // gflib; for StringLength

// check constants:
#if PLAYER_NAME_LENGTH != 7
   #error Constant `PLAYER_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if TRAINER_ID_LENGTH != 4
   #error Constant `TRAINER_ID_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if DECOR_MAX_SECRET_BASE != 16
   #error Constant `DECOR_MAX_SECRET_BASE` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_SecretBase(struct lu_BitstreamState* state, struct SecretBase* v) {
   v->secretBaseId = lu_BitstreamRead_u8(state, 8);
   v->toRegister = lu_BitstreamRead_u8(state, 4);
   v->gender = lu_BitstreamRead_bool(state);
   v->battledOwnerToday = lu_BitstreamRead_bool(state);
   v->registryStatus = lu_BitstreamRead_u8(state, 2);
   lu_BitstreamRead_string_optional_terminator(state, v->trainerName, PLAYER_NAME_LENGTH);
   {
      u16 i;
      for (i = 0; i < TRAINER_ID_LENGTH; ++i) { 
         v->trainerId[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   v->language = lu_BitstreamRead_u8(state, 3) + 0;
   v->numSecretBasesReceived = lu_BitstreamRead_u16(state, 16);
   v->numTimesEntered = lu_BitstreamRead_u8(state, 8);
   v->unused = lu_BitstreamRead_u8(state, 8);
   {
      u16 i;
      for (i = 0; i < DECOR_MAX_SECRET_BASE; ++i) { 
         v->decorations[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < DECOR_MAX_SECRET_BASE; ++i) { 
         v->decorationPositions[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   lu_BitstreamRead_SecretBaseParty(state, &v->party);
}

void lu_BitstreamWrite_SecretBase(struct lu_BitstreamState* state, const struct SecretBase* v) {
   lu_BitstreamWrite_u8(state, v->secretBaseId, 8);
   lu_BitstreamWrite_u8(state, v->toRegister, 4);
   lu_BitstreamWrite_bool(state, v->gender);
   lu_BitstreamWrite_bool(state, v->battledOwnerToday);
   lu_BitstreamWrite_u8(state, v->registryStatus, 2);
   lu_BitstreamWrite_string_optional_terminator(state, v->trainerName, PLAYER_NAME_LENGTH);
   {
      u16 i;
      for (i = 0; i < TRAINER_ID_LENGTH; ++i) { 
         lu_BitstreamWrite_u8(state, v->trainerId[i], 8);
      }
   }
   lu_BitstreamWrite_u8(state, v->language - 0, 3);
   lu_BitstreamWrite_u16(state, v->numSecretBasesReceived, 16);
   lu_BitstreamWrite_u8(state, v->numTimesEntered, 8);
   lu_BitstreamWrite_u8(state, v->unused, 8);
   {
      u16 i;
      for (i = 0; i < DECOR_MAX_SECRET_BASE; ++i) { 
         lu_BitstreamWrite_u8(state, v->decorations[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < DECOR_MAX_SECRET_BASE; ++i) { 
         lu_BitstreamWrite_u8(state, v->decorationPositions[i], 8);
      }
   }
   lu_BitstreamWrite_SecretBaseParty(state, &v->party);
}
