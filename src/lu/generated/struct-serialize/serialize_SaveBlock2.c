#include "lu/generated/struct-serialize//serialize_SaveBlock2.h"

#include "global.h"

#include "lu/bitstreams.h"

// dependencies
#include "lu/generated/struct-serialize//serialize_Pokedex.h"
#include "lu/generated/struct-serialize//serialize_Time.h"
#include "lu/generated/struct-serialize//serialize_Time.h"
#include "lu/generated/struct-serialize//serialize_PlayersApprentice.h"
#include "lu/generated/struct-serialize//serialize_Apprentice.h"
#include "lu/generated/struct-serialize//serialize_BerryCrush.h"
#include "lu/generated/struct-serialize//serialize_PokemonJumpRecords.h"
#include "lu/generated/struct-serialize//serialize_BerryPickingResults.h"
#include "lu/generated/struct-serialize//serialize_RankingHall1P.h"
#include "lu/generated/struct-serialize//serialize_RankingHall2P.h"
#include "lu/generated/struct-serialize//serialize_BattleFrontier.h"

#include "string_util.h" // gflib; for StringLength

// check constants:
#if PLAYER_NAME_LENGTH != 7
   #error Constant `PLAYER_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if TRAINER_ID_LENGTH != 4
   #error Constant `TRAINER_ID_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if APPRENTICE_COUNT != 4
   #error Constant `APPRENTICE_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if HALL_FACILITIES_COUNT != 9
   #error Constant `HALL_FACILITIES_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if FRONTIER_LVL_MODE_COUNT != 2
   #error Constant `FRONTIER_LVL_MODE_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if HALL_RECORDS_COUNT != 3
   #error Constant `HALL_RECORDS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if FRONTIER_LVL_MODE_COUNT != 2
   #error Constant `FRONTIER_LVL_MODE_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if HALL_RECORDS_COUNT != 3
   #error Constant `HALL_RECORDS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if CONTEST_CATEGORIES_COUNT != 5
   #error Constant `CONTEST_CATEGORIES_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if CONTESTANT_COUNT != 4
   #error Constant `CONTESTANT_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_SaveBlock2(struct lu_BitstreamState* state, struct SaveBlock2* v) {
   lu_BitstreamRead_string(state, v->playerName, PLAYER_NAME_LENGTH, 3);
   v->playerGender = lu_BitstreamRead_bool(state);
   v->specialSaveWarpFlags = lu_BitstreamRead_u8(state, 8);
   {
      u16 i;
      for (i = 0; i < TRAINER_ID_LENGTH; ++i) { 
         v->playerTrainerId[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   v->playTimeHours = lu_BitstreamRead_u16(state, 16);
   v->playTimeMinutes = lu_BitstreamRead_u8(state, 6) + 0;
   v->playTimeSeconds = lu_BitstreamRead_u8(state, 6) + 0;
   v->playTimeVBlanks = lu_BitstreamRead_u8(state, 8);
   v->optionsButtonMode = lu_BitstreamRead_u8(state, 2) + 0;
   v->optionsTextSpeed = lu_BitstreamRead_u8(state, 3);
   v->optionsWindowFrameType = lu_BitstreamRead_u8(state, 5);
   v->optionsSound = lu_BitstreamRead_bool(state);
   v->optionsBattleStyle = lu_BitstreamRead_bool(state);
   v->optionsBattleSceneOff = lu_BitstreamRead_bool(state);
   v->regionMapZoom = lu_BitstreamRead_bool(state);
   v->optionsRunningToggle = lu_BitstreamRead_bool(state);
   lu_BitstreamRead_Pokedex(state, &v->pokedex);
   lu_BitstreamRead_Time(state, &v->localTimeOffset);
   lu_BitstreamRead_Time(state, &v->lastBerryTreeUpdate);
   v->gcnLinkFlags = lu_BitstreamRead_u32(state, 32);
   v->encryptionKey = lu_BitstreamRead_u32(state, 32);
   lu_BitstreamRead_PlayersApprentice(state, &v->playerApprentice);
   {
      u16 i;
      for (i = 0; i < APPRENTICE_COUNT; ++i) { 
         lu_BitstreamRead_Apprentice(state, &v->apprentices[i]);
      }
   }
   lu_BitstreamRead_BerryCrush(state, &v->berryCrush);
   lu_BitstreamRead_PokemonJumpRecords(state, &v->pokeJump);
   lu_BitstreamRead_BerryPickingResults(state, &v->berryPick);
   {
      u16 i, j, k;
      for (i = 0; i < HALL_FACILITIES_COUNT; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            for (k = 0; k < HALL_RECORDS_COUNT; ++k) { 
               lu_BitstreamRead_RankingHall1P(state, &v->hallRecords1P[i][j][k]);
            }
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < FRONTIER_LVL_MODE_COUNT; ++i) { 
         for (j = 0; j < HALL_RECORDS_COUNT; ++j) { 
            lu_BitstreamRead_RankingHall2P(state, &v->hallRecords2P[i][j]);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < CONTEST_CATEGORIES_COUNT; ++i) { 
         for (j = 0; j < CONTESTANT_COUNT; ++j) { 
            v->contestLinkResults[i][j] = lu_BitstreamRead_u16(state, 16);
         }
      }
   }
   lu_BitstreamRead_BattleFrontier(state, &v->frontier);
}

void lu_BitstreamWrite_SaveBlock2(struct lu_BitstreamState* state, const struct SaveBlock2* v) {
   lu_BitstreamWrite_string(state, v->playerName, PLAYER_NAME_LENGTH, 3);
   lu_BitstreamWrite_bool(state, v->playerGender);
   lu_BitstreamWrite_u8(state, v->specialSaveWarpFlags, 8);
   {
      u16 i;
      for (i = 0; i < TRAINER_ID_LENGTH; ++i) { 
         lu_BitstreamWrite_u8(state, v->playerTrainerId[i], 8);
      }
   }
   lu_BitstreamWrite_u16(state, v->playTimeHours, 16);
   lu_BitstreamWrite_u8(state, v->playTimeMinutes, 6);
   lu_BitstreamWrite_u8(state, v->playTimeSeconds, 6);
   lu_BitstreamWrite_u8(state, v->playTimeVBlanks, 8);
   lu_BitstreamWrite_u8(state, v->optionsButtonMode, 2);
   lu_BitstreamWrite_u8(state, v->optionsTextSpeed, 3);
   lu_BitstreamWrite_u8(state, v->optionsWindowFrameType, 5);
   lu_BitstreamWrite_bool(state, v->optionsSound);
   lu_BitstreamWrite_bool(state, v->optionsBattleStyle);
   lu_BitstreamWrite_bool(state, v->optionsBattleSceneOff);
   lu_BitstreamWrite_bool(state, v->regionMapZoom);
   lu_BitstreamWrite_bool(state, v->optionsRunningToggle);
   lu_BitstreamWrite_Pokedex(state, &v->pokedex);
   lu_BitstreamWrite_Time(state, &v->localTimeOffset);
   lu_BitstreamWrite_Time(state, &v->lastBerryTreeUpdate);
   lu_BitstreamWrite_u32(state, v->gcnLinkFlags, 32);
   lu_BitstreamWrite_u32(state, v->encryptionKey, 32);
   lu_BitstreamWrite_PlayersApprentice(state, &v->playerApprentice);
   {
      u16 i;
      for (i = 0; i < APPRENTICE_COUNT; ++i) { 
         lu_BitstreamWrite_Apprentice(state, &v->apprentices[i]);
      }
   }
   lu_BitstreamWrite_BerryCrush(state, &v->berryCrush);
   lu_BitstreamWrite_PokemonJumpRecords(state, &v->pokeJump);
   lu_BitstreamWrite_BerryPickingResults(state, &v->berryPick);
   {
      u16 i, j, k;
      for (i = 0; i < HALL_FACILITIES_COUNT; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            for (k = 0; k < HALL_RECORDS_COUNT; ++k) { 
               lu_BitstreamWrite_RankingHall1P(state, &v->hallRecords1P[i][j][k]);
            }
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < FRONTIER_LVL_MODE_COUNT; ++i) { 
         for (j = 0; j < HALL_RECORDS_COUNT; ++j) { 
            lu_BitstreamWrite_RankingHall2P(state, &v->hallRecords2P[i][j]);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < CONTEST_CATEGORIES_COUNT; ++i) { 
         for (j = 0; j < CONTESTANT_COUNT; ++j) { 
            lu_BitstreamWrite_u16(state, v->contestLinkResults[i][j], 16);
         }
      }
   }
   lu_BitstreamWrite_BattleFrontier(state, &v->frontier);
}
