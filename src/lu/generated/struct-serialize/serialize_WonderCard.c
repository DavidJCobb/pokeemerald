#include "lu/generated/struct-serialize//serialize_WonderCard.h"

#include "global.h"

#include "lu/bitstreams.h"

#include "string_util.h" // gflib; for StringLength

// check constants:
#if WONDER_CARD_TEXT_LENGTH != 40
   #error Constant `WONDER_CARD_TEXT_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if WONDER_CARD_BODY_TEXT_LINES != 4
   #error Constant `WONDER_CARD_BODY_TEXT_LINES` changed in C, but XML not updated or codegen not re-run!
#endif
#if WONDER_CARD_TEXT_LENGTH != 40
   #error Constant `WONDER_CARD_TEXT_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_WonderCard(struct lu_BitstreamState* state, struct WonderCard* v) {
   v->flagId = lu_BitstreamRead_u16(state, 16);
   v->iconSpecies = lu_BitstreamRead_u16(state, 11) + 0;
   v->idNumber = lu_BitstreamRead_u32(state, 32);
   v->type = lu_BitstreamRead_u8(state, 2);
   v->bgType = lu_BitstreamRead_u8(state, 4);
   v->sendType = lu_BitstreamRead_u8(state, 2);
   v->maxStamps = lu_BitstreamRead_u8(state, 8);
   lu_BitstreamRead_string_optional_terminator(state, v->titleText, WONDER_CARD_TEXT_LENGTH);
   lu_BitstreamRead_string_optional_terminator(state, v->subtitleText, WONDER_CARD_TEXT_LENGTH);
   {
      u16 i;
      for (i = 0; i < WONDER_CARD_BODY_TEXT_LINES; ++i) { 
         lu_BitstreamRead_string_optional_terminator(state, v->bodyText[i], WONDER_CARD_TEXT_LENGTH);
      }
   }
   lu_BitstreamRead_string_optional_terminator(state, v->footerLine1Text, WONDER_CARD_TEXT_LENGTH);
   lu_BitstreamRead_string_optional_terminator(state, v->footerLine2Text, WONDER_CARD_TEXT_LENGTH);
}

void lu_BitstreamWrite_WonderCard(struct lu_BitstreamState* state, const struct WonderCard* v) {
   lu_BitstreamWrite_u16(state, v->flagId, 16);
   lu_BitstreamWrite_u16(state, v->iconSpecies, 11);
   lu_BitstreamWrite_u32(state, v->idNumber, 32);
   lu_BitstreamWrite_u8(state, v->type, 2);
   lu_BitstreamWrite_u8(state, v->bgType, 4);
   lu_BitstreamWrite_u8(state, v->sendType, 2);
   lu_BitstreamWrite_u8(state, v->maxStamps, 8);
   lu_BitstreamWrite_string_optional_terminator(state, v->titleText, WONDER_CARD_TEXT_LENGTH);
   lu_BitstreamWrite_string_optional_terminator(state, v->subtitleText, WONDER_CARD_TEXT_LENGTH);
   {
      u16 i;
      for (i = 0; i < WONDER_CARD_BODY_TEXT_LINES; ++i) { 
         lu_BitstreamWrite_string_optional_terminator(state, v->bodyText[i], WONDER_CARD_TEXT_LENGTH);
      }
   }
   lu_BitstreamWrite_string_optional_terminator(state, v->footerLine1Text, WONDER_CARD_TEXT_LENGTH);
   lu_BitstreamWrite_string_optional_terminator(state, v->footerLine2Text, WONDER_CARD_TEXT_LENGTH);
}
