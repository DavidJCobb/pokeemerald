#include "lu/generated/struct-serialize//serialize_BattleFrontier.h"

#include "global.h"

#include "lu/bitstreams.h"

// dependencies
#include "lu/generated/struct-serialize//serialize_EmeraldBattleTowerRecord.h"
#include "lu/generated/struct-serialize//serialize_EmeraldBattleTowerRecord.h"
#include "lu/generated/struct-serialize//serialize_BattleTowerInterview.h"
#include "lu/generated/struct-serialize//serialize_BattleTowerEReaderTrainer.h"
#include "lu/generated/struct-serialize//serialize_BattleDomeTrainer.h"
#include "lu/generated/struct-serialize//serialize_PyramidBag.h"
#include "lu/generated/struct-serialize//serialize_RentalMon.h"
#include "lu/generated/struct-serialize//serialize_DomeMonData.h"

#include "string_util.h" // gflib; for StringLength

// check constants:
#if BATTLE_TOWER_RECORD_COUNT != 5
   #error Constant `BATTLE_TOWER_RECORD_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if MAX_FRONTIER_PARTY_SIZE != 4
   #error Constant `MAX_FRONTIER_PARTY_SIZE` changed in C, but XML not updated or codegen not re-run!
#endif
#if FRONTIER_LVL_MODE_COUNT != 2
   #error Constant `FRONTIER_LVL_MODE_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if DOME_TOURNAMENT_TRAINERS_COUNT != 16
   #error Constant `DOME_TOURNAMENT_TRAINERS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if FRONTIER_PARTY_SIZE != 3
   #error Constant `FRONTIER_PARTY_SIZE` changed in C, but XML not updated or codegen not re-run!
#endif
#if FRONTIER_LVL_MODE_COUNT != 2
   #error Constant `FRONTIER_LVL_MODE_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if FRONTIER_PARTY_SIZE != 3
   #error Constant `FRONTIER_PARTY_SIZE` changed in C, but XML not updated or codegen not re-run!
#endif
#if FRONTIER_LVL_MODE_COUNT != 2
   #error Constant `FRONTIER_LVL_MODE_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if DOME_TOURNAMENT_TRAINERS_COUNT != 16
   #error Constant `DOME_TOURNAMENT_TRAINERS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if PLAYER_NAME_LENGTH != 7
   #error Constant `PLAYER_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if FRONTIER_LVL_MODE_COUNT != 2
   #error Constant `FRONTIER_LVL_MODE_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if TRAINER_ID_LENGTH != 4
   #error Constant `TRAINER_ID_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if FRONTIER_PARTY_SIZE != 3
   #error Constant `FRONTIER_PARTY_SIZE` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_BattleFrontier(struct lu_BitstreamState* state, struct BattleFrontier* v) {
   lu_BitstreamRead_EmeraldBattleTowerRecord(state, &v->towerPlayer);
   {
      u16 i;
      for (i = 0; i < BATTLE_TOWER_RECORD_COUNT; ++i) { 
         lu_BitstreamRead_EmeraldBattleTowerRecord(state, &v->towerRecords[i]);
      }
   }
   lu_BitstreamRead_BattleTowerInterview(state, &v->towerInterview);
   lu_BitstreamRead_BattleTowerEReaderTrainer(state, &v->ereaderTrainer);
   v->challengeStatus = lu_BitstreamRead_u8(state, 8);
   v->lvlMode = lu_BitstreamRead_u8(state, 2);
   v->challengePaused = lu_BitstreamRead_bool(state);
   v->disableRecordBattle = lu_BitstreamRead_bool(state);
   {
      u16 i;
      for (i = 0; i < MAX_FRONTIER_PARTY_SIZE; ++i) { 
         v->selectedPartyMons[i] = lu_BitstreamRead_u16(state, 11) + 0;
      }
   }
   v->curChallengeBattleNum = lu_BitstreamRead_u16(state, 16);
   {
      u16 i;
      for (i = 0; i < 20; ++i) { 
         v->trainerIds[i] = lu_BitstreamRead_u16(state, 16);
      }
   }
   v->winStreakActiveFlags = lu_BitstreamRead_u32(state, 32);
   {
      u16 i, j;
      for (i = 0; i < 4; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            v->towerWinStreaks[i][j] = lu_BitstreamRead_u16(state, 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 4; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            v->towerRecordWinStreaks[i][j] = lu_BitstreamRead_u16(state, 16);
         }
      }
   }
   v->battledBrainFlags = lu_BitstreamRead_u16(state, 16);
   v->towerSinglesStreak = lu_BitstreamRead_u16(state, 16);
   v->towerNumWins = lu_BitstreamRead_u16(state, 16);
   v->towerBattleOutcome = lu_BitstreamRead_u8(state, 8);
   v->towerLvlMode = lu_BitstreamRead_u8(state, 8);
   v->domeAttemptedSingles50 = lu_BitstreamRead_bool(state);
   v->domeAttemptedSinglesOpen = lu_BitstreamRead_bool(state);
   v->domeHasWonSingles50 = lu_BitstreamRead_bool(state);
   v->domeHasWonSinglesOpen = lu_BitstreamRead_bool(state);
   v->domeAttemptedDoubles50 = lu_BitstreamRead_bool(state);
   v->domeAttemptedDoublesOpen = lu_BitstreamRead_bool(state);
   v->domeHasWonDoubles50 = lu_BitstreamRead_bool(state);
   v->domeHasWonDoublesOpen = lu_BitstreamRead_bool(state);
   v->domeUnused = lu_BitstreamRead_u8(state, 8);
   v->domeLvlMode = lu_BitstreamRead_u8(state, 8);
   v->domeBattleMode = lu_BitstreamRead_u8(state, 8);
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            v->domeWinStreaks[i][j] = lu_BitstreamRead_u16(state, 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            v->domeRecordWinStreaks[i][j] = lu_BitstreamRead_u16(state, 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            v->domeTotalChampionships[i][j] = lu_BitstreamRead_u16(state, 16);
         }
      }
   }
   {
      u16 i;
      for (i = 0; i < DOME_TOURNAMENT_TRAINERS_COUNT; ++i) { 
         lu_BitstreamRead_BattleDomeTrainer(state, &v->domeTrainers[i]);
      }
   }
   {
      u16 i, j;
      for (i = 0; i < DOME_TOURNAMENT_TRAINERS_COUNT; ++i) { 
         for (j = 0; j < FRONTIER_PARTY_SIZE; ++j) { 
            v->domeMonIds[i][j] = lu_BitstreamRead_u16(state, 11) + 0;
         }
      }
   }
   v->unused_DC4 = lu_BitstreamRead_u16(state, 16);
   v->palacePrice = lu_BitstreamRead_u16(state, 16);
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            v->palaceWinStreaks[i][j] = lu_BitstreamRead_u16(state, 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            v->palaceRecordWinStreaks[i][j] = lu_BitstreamRead_u16(state, 16);
         }
      }
   }
   v->arenaPrize = lu_BitstreamRead_u16(state, 16);
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            v->arenaWinStreaks[i][j] = lu_BitstreamRead_u16(state, 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            v->arenaRecordStreaks[i][j] = lu_BitstreamRead_u16(state, 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            v->factoryWinStreaks[i][j] = lu_BitstreamRead_u16(state, 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            v->factoryRecordWinStreaks[i][j] = lu_BitstreamRead_u16(state, 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            v->factoryRentsCount[i][j] = lu_BitstreamRead_u16(state, 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            v->factoryRecordRentsCount[i][j] = lu_BitstreamRead_u16(state, 16);
         }
      }
   }
   v->pikePrize = lu_BitstreamRead_u16(state, 16);
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            v->pikeWinStreaks[i][j] = lu_BitstreamRead_u16(state, 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            v->pikeRecordStreaks[i][j] = lu_BitstreamRead_u16(state, 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            v->pikeTotalStreaks[i][j] = lu_BitstreamRead_u16(state, 16);
         }
      }
   }
   v->pikeHintedRoomIndex = lu_BitstreamRead_u8(state, 3);
   v->pikeHintedRoomType = lu_BitstreamRead_u8(state, 4);
   v->pikeHealingRoomsDisabled = lu_BitstreamRead_bool(state);
   {
      u16 i;
      for (i = 0; i < FRONTIER_PARTY_SIZE; ++i) { 
         v->pikeHeldItemsBackup[i] = lu_BitstreamRead_u16(state, 9);
      }
   }
   v->pyramidPrize = lu_BitstreamRead_u16(state, 16);
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            v->pyramidWinStreaks[i][j] = lu_BitstreamRead_u16(state, 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            v->pyramidRecordStreaks[i][j] = lu_BitstreamRead_u16(state, 16);
         }
      }
   }
   {
      u16 i;
      for (i = 0; i < 4; ++i) { 
         v->pyramidRandoms[i] = lu_BitstreamRead_u16(state, 16);
      }
   }
   v->pyramidTrainerFlags = lu_BitstreamRead_u8(state, 8);
   lu_BitstreamRead_PyramidBag(state, &v->pyramidBag);
   v->pyramidLightRadius = lu_BitstreamRead_u8(state, 8);
   v->verdanturfTentPrize = lu_BitstreamRead_u16(state, 16);
   v->fallarborTentPrize = lu_BitstreamRead_u16(state, 16);
   v->slateportTentPrize = lu_BitstreamRead_u16(state, 16);
   {
      u16 i;
      for (i = 0; i < FRONTIER_PARTY_SIZE * 2; ++i) { 
         lu_BitstreamRead_RentalMon(state, &v->rentalMons[i]);
      }
   }
   v->battlePoints = lu_BitstreamRead_u16(state, 16);
   v->cardBattlePoints = lu_BitstreamRead_u16(state, 16);
   v->battlesCount = lu_BitstreamRead_u32(state, 32);
   {
      u16 i;
      for (i = 0; i < DOME_TOURNAMENT_TRAINERS_COUNT; ++i) { 
         v->domeWinningMoves[i] = lu_BitstreamRead_u16(state, 16);
      }
   }
   v->trainerFlags = lu_BitstreamRead_u8(state, 8);
   {
      u16 i;
      for (i = 0; i < FRONTIER_LVL_MODE_COUNT; ++i) { 
         lu_BitstreamRead_string(state, v->opponentNames[i], PLAYER_NAME_LENGTH, 3);
      }
   }
   {
      u16 i, j;
      for (i = 0; i < FRONTIER_LVL_MODE_COUNT; ++i) { 
         for (j = 0; j < TRAINER_ID_LENGTH; ++j) { 
            v->opponentTrainerIds[i][j] = lu_BitstreamRead_u8(state, 8);
         }
      }
   }
   v->unk_EF9 = lu_BitstreamRead_u8(state, 7);
   v->savedGame = lu_BitstreamRead_bool(state);
   v->unused_EFA = lu_BitstreamRead_u8(state, 8);
   v->unused_EFB = lu_BitstreamRead_u8(state, 8);
   {
      u16 i;
      for (i = 0; i < FRONTIER_PARTY_SIZE; ++i) { 
         lu_BitstreamRead_DomeMonData(state, &v->domePlayerPartyData[i]);
      }
   }
}

void lu_BitstreamWrite_BattleFrontier(struct lu_BitstreamState* state, const struct BattleFrontier* v) {
   lu_BitstreamWrite_EmeraldBattleTowerRecord(state, &v->towerPlayer);
   {
      u16 i;
      for (i = 0; i < BATTLE_TOWER_RECORD_COUNT; ++i) { 
         lu_BitstreamWrite_EmeraldBattleTowerRecord(state, &v->towerRecords[i]);
      }
   }
   lu_BitstreamWrite_BattleTowerInterview(state, &v->towerInterview);
   lu_BitstreamWrite_BattleTowerEReaderTrainer(state, &v->ereaderTrainer);
   lu_BitstreamWrite_u8(state, v->challengeStatus, 8);
   lu_BitstreamWrite_u8(state, v->lvlMode, 2);
   lu_BitstreamWrite_bool(state, v->challengePaused);
   lu_BitstreamWrite_bool(state, v->disableRecordBattle);
   {
      u16 i;
      for (i = 0; i < MAX_FRONTIER_PARTY_SIZE; ++i) { 
         lu_BitstreamWrite_u16(state, v->selectedPartyMons[i], 11);
      }
   }
   lu_BitstreamWrite_u16(state, v->curChallengeBattleNum, 16);
   {
      u16 i;
      for (i = 0; i < 20; ++i) { 
         lu_BitstreamWrite_u16(state, v->trainerIds[i], 16);
      }
   }
   lu_BitstreamWrite_u32(state, v->winStreakActiveFlags, 32);
   {
      u16 i, j;
      for (i = 0; i < 4; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            lu_BitstreamWrite_u16(state, v->towerWinStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 4; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            lu_BitstreamWrite_u16(state, v->towerRecordWinStreaks[i][j], 16);
         }
      }
   }
   lu_BitstreamWrite_u16(state, v->battledBrainFlags, 16);
   lu_BitstreamWrite_u16(state, v->towerSinglesStreak, 16);
   lu_BitstreamWrite_u16(state, v->towerNumWins, 16);
   lu_BitstreamWrite_u8(state, v->towerBattleOutcome, 8);
   lu_BitstreamWrite_u8(state, v->towerLvlMode, 8);
   lu_BitstreamWrite_bool(state, v->domeAttemptedSingles50);
   lu_BitstreamWrite_bool(state, v->domeAttemptedSinglesOpen);
   lu_BitstreamWrite_bool(state, v->domeHasWonSingles50);
   lu_BitstreamWrite_bool(state, v->domeHasWonSinglesOpen);
   lu_BitstreamWrite_bool(state, v->domeAttemptedDoubles50);
   lu_BitstreamWrite_bool(state, v->domeAttemptedDoublesOpen);
   lu_BitstreamWrite_bool(state, v->domeHasWonDoubles50);
   lu_BitstreamWrite_bool(state, v->domeHasWonDoublesOpen);
   lu_BitstreamWrite_u8(state, v->domeUnused, 8);
   lu_BitstreamWrite_u8(state, v->domeLvlMode, 8);
   lu_BitstreamWrite_u8(state, v->domeBattleMode, 8);
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            lu_BitstreamWrite_u16(state, v->domeWinStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            lu_BitstreamWrite_u16(state, v->domeRecordWinStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            lu_BitstreamWrite_u16(state, v->domeTotalChampionships[i][j], 16);
         }
      }
   }
   {
      u16 i;
      for (i = 0; i < DOME_TOURNAMENT_TRAINERS_COUNT; ++i) { 
         lu_BitstreamWrite_BattleDomeTrainer(state, &v->domeTrainers[i]);
      }
   }
   {
      u16 i, j;
      for (i = 0; i < DOME_TOURNAMENT_TRAINERS_COUNT; ++i) { 
         for (j = 0; j < FRONTIER_PARTY_SIZE; ++j) { 
            lu_BitstreamWrite_u16(state, v->domeMonIds[i][j], 11);
         }
      }
   }
   lu_BitstreamWrite_u16(state, v->unused_DC4, 16);
   lu_BitstreamWrite_u16(state, v->palacePrice, 16);
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            lu_BitstreamWrite_u16(state, v->palaceWinStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            lu_BitstreamWrite_u16(state, v->palaceRecordWinStreaks[i][j], 16);
         }
      }
   }
   lu_BitstreamWrite_u16(state, v->arenaPrize, 16);
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            lu_BitstreamWrite_u16(state, v->arenaWinStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            lu_BitstreamWrite_u16(state, v->arenaRecordStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            lu_BitstreamWrite_u16(state, v->factoryWinStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            lu_BitstreamWrite_u16(state, v->factoryRecordWinStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            lu_BitstreamWrite_u16(state, v->factoryRentsCount[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            lu_BitstreamWrite_u16(state, v->factoryRecordRentsCount[i][j], 16);
         }
      }
   }
   lu_BitstreamWrite_u16(state, v->pikePrize, 16);
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            lu_BitstreamWrite_u16(state, v->pikeWinStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            lu_BitstreamWrite_u16(state, v->pikeRecordStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            lu_BitstreamWrite_u16(state, v->pikeTotalStreaks[i][j], 16);
         }
      }
   }
   lu_BitstreamWrite_u8(state, v->pikeHintedRoomIndex, 3);
   lu_BitstreamWrite_u8(state, v->pikeHintedRoomType, 4);
   lu_BitstreamWrite_bool(state, v->pikeHealingRoomsDisabled);
   {
      u16 i;
      for (i = 0; i < FRONTIER_PARTY_SIZE; ++i) { 
         lu_BitstreamWrite_u16(state, v->pikeHeldItemsBackup[i], 9);
      }
   }
   lu_BitstreamWrite_u16(state, v->pyramidPrize, 16);
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            lu_BitstreamWrite_u16(state, v->pyramidWinStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            lu_BitstreamWrite_u16(state, v->pyramidRecordStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i;
      for (i = 0; i < 4; ++i) { 
         lu_BitstreamWrite_u16(state, v->pyramidRandoms[i], 16);
      }
   }
   lu_BitstreamWrite_u8(state, v->pyramidTrainerFlags, 8);
   lu_BitstreamWrite_PyramidBag(state, &v->pyramidBag);
   lu_BitstreamWrite_u8(state, v->pyramidLightRadius, 8);
   lu_BitstreamWrite_u16(state, v->verdanturfTentPrize, 16);
   lu_BitstreamWrite_u16(state, v->fallarborTentPrize, 16);
   lu_BitstreamWrite_u16(state, v->slateportTentPrize, 16);
   {
      u16 i;
      for (i = 0; i < FRONTIER_PARTY_SIZE * 2; ++i) { 
         lu_BitstreamWrite_RentalMon(state, &v->rentalMons[i]);
      }
   }
   lu_BitstreamWrite_u16(state, v->battlePoints, 16);
   lu_BitstreamWrite_u16(state, v->cardBattlePoints, 16);
   lu_BitstreamWrite_u32(state, v->battlesCount, 32);
   {
      u16 i;
      for (i = 0; i < DOME_TOURNAMENT_TRAINERS_COUNT; ++i) { 
         lu_BitstreamWrite_u16(state, v->domeWinningMoves[i], 16);
      }
   }
   lu_BitstreamWrite_u8(state, v->trainerFlags, 8);
   {
      u16 i;
      for (i = 0; i < FRONTIER_LVL_MODE_COUNT; ++i) { 
         lu_BitstreamWrite_string(state, v->opponentNames[i], PLAYER_NAME_LENGTH, 3);
      }
   }
   {
      u16 i, j;
      for (i = 0; i < FRONTIER_LVL_MODE_COUNT; ++i) { 
         for (j = 0; j < TRAINER_ID_LENGTH; ++j) { 
            lu_BitstreamWrite_u8(state, v->opponentTrainerIds[i][j], 8);
         }
      }
   }
   lu_BitstreamWrite_u8(state, v->unk_EF9, 7);
   lu_BitstreamWrite_bool(state, v->savedGame);
   lu_BitstreamWrite_u8(state, v->unused_EFA, 8);
   lu_BitstreamWrite_u8(state, v->unused_EFB, 8);
   {
      u16 i;
      for (i = 0; i < FRONTIER_PARTY_SIZE; ++i) { 
         lu_BitstreamWrite_DomeMonData(state, &v->domePlayerPartyData[i]);
      }
   }
}
