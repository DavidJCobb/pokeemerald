#include "lu/generated/struct-serialize//serialize_MysteryGiftSave.h"

#include "global.h"

#include "lu/bitstreams.h"

// dependencies
#include "lu/generated/struct-serialize//serialize_WonderNews.h"
#include "lu/generated/struct-serialize//serialize_WonderCard.h"
#include "lu/generated/struct-serialize//serialize_WonderCardMetadata.h"
#include "lu/generated/struct-serialize//serialize_WonderNewsMetadata.h"

// check constants:
#if NUM_QUESTIONNAIRE_WORDS != 4
   #error Constant `NUM_QUESTIONNAIRE_WORDS` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_MysteryGiftSave(struct lu_BitstreamState* state, struct MysteryGiftSave* v) {
   v->newsCrc = lu_BitstreamRead_u32(state, 32);
   lu_BitstreamRead_WonderNews(state, &v->news);
   v->cardCrc = lu_BitstreamRead_u32(state, 32);
   lu_BitstreamRead_WonderCard(state, &v->card);
   v->cardMetadataCrc = lu_BitstreamRead_u32(state, 32);
   lu_BitstreamRead_WonderCardMetadata(state, &v->cardMetadata);
   {
      u16 i;
      for (i = 0; i < NUM_QUESTIONNAIRE_WORDS; ++i) { 
         v->questionnaireWords[i] = lu_BitstreamRead_u16(state, 16);
      }
   }
   lu_BitstreamRead_WonderNewsMetadata(state, &v->newsMetadata);
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < 5; ++j) { 
            v->trainerIds[i][j] = lu_BitstreamRead_u32(state, 32);
         }
      }
   }
}

void lu_BitstreamWrite_MysteryGiftSave(struct lu_BitstreamState* state, const struct MysteryGiftSave* v) {
   lu_BitstreamWrite_u32(state, v->newsCrc, 32);
   lu_BitstreamWrite_WonderNews(state, &v->news);
   lu_BitstreamWrite_u32(state, v->cardCrc, 32);
   lu_BitstreamWrite_WonderCard(state, &v->card);
   lu_BitstreamWrite_u32(state, v->cardMetadataCrc, 32);
   lu_BitstreamWrite_WonderCardMetadata(state, &v->cardMetadata);
   {
      u16 i;
      for (i = 0; i < NUM_QUESTIONNAIRE_WORDS; ++i) { 
         lu_BitstreamWrite_u16(state, v->questionnaireWords[i], 16);
      }
   }
   lu_BitstreamWrite_WonderNewsMetadata(state, &v->newsMetadata);
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < 5; ++j) { 
            lu_BitstreamWrite_u32(state, v->trainerIds[i][j], 32);
         }
      }
   }
}
