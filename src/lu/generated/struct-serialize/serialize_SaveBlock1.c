#include "lu/generated/struct-serialize//serialize_SaveBlock1.h"

#include "global.h"

#include "lu/bitstreams.h"

// dependencies
#include "lu/generated/struct-serialize//serialize_Coords16.h"
#include "lu/generated/struct-serialize//serialize_WarpData.h"
#include "lu/generated/struct-serialize//serialize_WarpData.h"
#include "lu/generated/struct-serialize//serialize_WarpData.h"
#include "lu/generated/struct-serialize//serialize_WarpData.h"
#include "lu/generated/struct-serialize//serialize_WarpData.h"
#include "lu/generated/struct-serialize//serialize_Pokemon.h"
#include "lu/generated/struct-serialize//serialize_ItemSlot.h"
#include "lu/generated/struct-serialize//serialize_ItemSlot.h"
#include "lu/generated/struct-serialize//serialize_ItemSlot.h"
#include "lu/generated/struct-serialize//serialize_ItemSlot.h"
#include "lu/generated/struct-serialize//serialize_ItemSlot.h"
#include "lu/generated/struct-serialize//serialize_ItemSlot.h"
#include "lu/generated/struct-serialize//serialize_Pokeblock.h"
#include "lu/generated/struct-serialize//serialize_ObjectEvent.h"
#include "lu/generated/struct-serialize//serialize_ObjectEventTemplate.h"
#include "lu/generated/struct-serialize//serialize_BerryTree.h"
#include "lu/generated/struct-serialize//serialize_SecretBase.h"
#include "lu/generated/struct-serialize//serialize_PokeNews.h"
#include "lu/generated/struct-serialize//serialize_GabbyAndTyData.h"
#include "lu/generated/struct-serialize//serialize_Mail.h"
#include "lu/generated/struct-serialize//serialize_DewfordTrend.h"
#include "lu/generated/struct-serialize//serialize_ContestWinner.h"
#include "lu/generated/struct-serialize//serialize_DayCare.h"
#include "lu/generated/struct-serialize//serialize_LinkBattleRecords.h"
#include "lu/generated/struct-serialize//serialize_ExternalEventData.h"
#include "lu/generated/struct-serialize//serialize_ExternalEventFlags.h"
#include "lu/generated/struct-serialize//serialize_Roamer.h"
#include "lu/generated/struct-serialize//serialize_EnigmaBerry.h"
#include "lu/generated/struct-serialize//serialize_MysteryGiftSave.h"
#include "lu/generated/struct-serialize//serialize_RamScript.h"
#include "lu/generated/struct-serialize//serialize_RecordMixingGift.h"
#include "lu/generated/struct-serialize//serialize_TrainerNameRecord.h"
#include "lu/generated/struct-serialize//serialize_TrainerHillSave.h"
#include "lu/generated/struct-serialize//serialize_WaldaPhrase.h"

// check constants:
#if PARTY_SIZE != 6
   #error Constant `PARTY_SIZE` changed in C, but XML not updated or codegen not re-run!
#endif
#if PC_ITEMS_COUNT != 50
   #error Constant `PC_ITEMS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if BAG_ITEMS_COUNT != 30
   #error Constant `BAG_ITEMS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if BAG_KEYITEMS_COUNT != 30
   #error Constant `BAG_KEYITEMS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if BAG_POKEBALLS_COUNT != 16
   #error Constant `BAG_POKEBALLS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if BAG_TMHM_COUNT != 64
   #error Constant `BAG_TMHM_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if BAG_BERRIES_COUNT != 46
   #error Constant `BAG_BERRIES_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if POKEBLOCKS_COUNT != 40
   #error Constant `POKEBLOCKS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if NUM_DEX_FLAG_BYTES != 52
   #error Constant `NUM_DEX_FLAG_BYTES` changed in C, but XML not updated or codegen not re-run!
#endif
#if MAX_REMATCH_ENTRIES != 100
   #error Constant `MAX_REMATCH_ENTRIES` changed in C, but XML not updated or codegen not re-run!
#endif
#if OBJECT_EVENTS_COUNT != 16
   #error Constant `OBJECT_EVENTS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if OBJECT_EVENT_TEMPLATES_COUNT != 64
   #error Constant `OBJECT_EVENT_TEMPLATES_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if NUM_FLAG_BYTES != 300
   #error Constant `NUM_FLAG_BYTES` changed in C, but XML not updated or codegen not re-run!
#endif
#if VARS_COUNT != 256
   #error Constant `VARS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if NUM_GAME_STATS != 64
   #error Constant `NUM_GAME_STATS` changed in C, but XML not updated or codegen not re-run!
#endif
#if BERRY_TREES_COUNT != 128
   #error Constant `BERRY_TREES_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if SECRET_BASES_COUNT != 20
   #error Constant `SECRET_BASES_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if DECOR_MAX_PLAYERS_HOUSE != 12
   #error Constant `DECOR_MAX_PLAYERS_HOUSE` changed in C, but XML not updated or codegen not re-run!
#endif
#if TV_SHOWS_COUNT != 25
   #error Constant `TV_SHOWS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if POKE_NEWS_COUNT != 16
   #error Constant `POKE_NEWS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if MAX_MON_MOVES != 4
   #error Constant `MAX_MON_MOVES` changed in C, but XML not updated or codegen not re-run!
#endif
#if EASY_CHAT_BATTLE_WORDS_COUNT != 6
   #error Constant `EASY_CHAT_BATTLE_WORDS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if MAIL_COUNT != 16
   #error Constant `MAIL_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if NUM_TRENDY_SAYING_BYTES != 5
   #error Constant `NUM_TRENDY_SAYING_BYTES` changed in C, but XML not updated or codegen not re-run!
#endif
#if SAVED_TRENDS_COUNT != 5
   #error Constant `SAVED_TRENDS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if NUM_CONTEST_WINNERS != 13
   #error Constant `NUM_CONTEST_WINNERS` changed in C, but XML not updated or codegen not re-run!
#endif
#if GIFT_RIBBONS_COUNT != 11
   #error Constant `GIFT_RIBBONS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if NUM_TRAINER_HILL_MODES != 4
   #error Constant `NUM_TRAINER_HILL_MODES` changed in C, but XML not updated or codegen not re-run!
#endif
#if NUM_DEX_FLAG_BYTES != 52
   #error Constant `NUM_DEX_FLAG_BYTES` changed in C, but XML not updated or codegen not re-run!
#endif
#if UNION_ROOM_KB_ROW_COUNT != 10
   #error Constant `UNION_ROOM_KB_ROW_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_SaveBlock1(struct lu_BitstreamState* state, struct SaveBlock1* v) {
   lu_BitstreamRead_Coords16(state, &v->pos);
   lu_BitstreamRead_WarpData(state, &v->location);
   lu_BitstreamRead_WarpData(state, &v->continueGameWarp);
   lu_BitstreamRead_WarpData(state, &v->dynamicWarp);
   lu_BitstreamRead_WarpData(state, &v->lastHealLocation);
   lu_BitstreamRead_WarpData(state, &v->escapeWarp);
   v->savedMusic = lu_BitstreamRead_u16(state, 16);
   v->weather = lu_BitstreamRead_u8(state, 8);
   v->weatherCycleStage = lu_BitstreamRead_u8(state, 8);
   v->flashLevel = lu_BitstreamRead_u8(state, 8);
   v->mapLayoutId = lu_BitstreamRead_u16(state, 16);
   {
      u16 i;
      for (i = 0; i < 0x100; ++i) { 
         v->mapView[i] = lu_BitstreamRead_u16(state, 16);
      }
   }
   v->playerPartyCount = lu_BitstreamRead_u8(state, 3) + 0;
   {
      u16 i;
      for (i = 0; i < PARTY_SIZE; ++i) { 
         lu_BitstreamRead_Pokemon(state, &v->playerParty[i]);
      }
   }
   v->money = lu_BitstreamRead_u32(state, 32);
   v->coins = lu_BitstreamRead_u16(state, 16);
   v->registeredItem = lu_BitstreamRead_u16(state, 9);
   {
      u16 i;
      for (i = 0; i < PC_ITEMS_COUNT; ++i) { 
         lu_BitstreamRead_ItemSlot(state, &v->pcItems[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < BAG_ITEMS_COUNT; ++i) { 
         lu_BitstreamRead_ItemSlot(state, &v->bagPocket_Items[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < BAG_KEYITEMS_COUNT; ++i) { 
         lu_BitstreamRead_ItemSlot(state, &v->bagPocket_KeyItems[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < BAG_POKEBALLS_COUNT; ++i) { 
         lu_BitstreamRead_ItemSlot(state, &v->bagPocket_PokeBalls[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < BAG_TMHM_COUNT; ++i) { 
         lu_BitstreamRead_ItemSlot(state, &v->bagPocket_TMHM[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < BAG_BERRIES_COUNT; ++i) { 
         lu_BitstreamRead_ItemSlot(state, &v->bagPocket_Berries[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < POKEBLOCKS_COUNT; ++i) { 
         lu_BitstreamRead_Pokeblock(state, &v->pokeblocks[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < NUM_DEX_FLAG_BYTES; ++i) { 
         v->seen1[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < 3; ++i) { 
         v->berryBlenderRecords[i] = lu_BitstreamRead_u16(state, 16);
      }
   }
   v->trainerRematchStepCounter = lu_BitstreamRead_u16(state, 16);
   {
      u16 i;
      for (i = 0; i < MAX_REMATCH_ENTRIES; ++i) { 
         v->trainerRematches[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < OBJECT_EVENTS_COUNT; ++i) { 
         lu_BitstreamRead_ObjectEvent(state, &v->objectEvents[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < OBJECT_EVENT_TEMPLATES_COUNT; ++i) { 
         lu_BitstreamRead_ObjectEventTemplate(state, &v->objectEventTemplates[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < NUM_FLAG_BYTES; ++i) { 
         v->flags[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < VARS_COUNT; ++i) { 
         v->vars[i] = lu_BitstreamRead_u16(state, 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < NUM_GAME_STATS; ++i) { 
         v->gameStats[i] = lu_BitstreamRead_u32(state, 32);
      }
   }
   {
      u16 i;
      for (i = 0; i < BERRY_TREES_COUNT; ++i) { 
         lu_BitstreamRead_BerryTree(state, &v->berryTrees[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < SECRET_BASES_COUNT; ++i) { 
         lu_BitstreamRead_SecretBase(state, &v->secretBases[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < DECOR_MAX_PLAYERS_HOUSE; ++i) { 
         v->playerRoomDecorations[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < DECOR_MAX_PLAYERS_HOUSE; ++i) { 
         v->playerRoomDecorationPositions[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < 10; ++i) { 
         v->decorationDesks[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < 10; ++i) { 
         v->decorationChairs[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < 10; ++i) { 
         v->decorationPlants[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < 30; ++i) { 
         v->decorationOrnaments[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < 30; ++i) { 
         v->decorationMats[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < 10; ++i) { 
         v->decorationPosters[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < 40; ++i) { 
         v->decorationDolls[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < 10; ++i) { 
         v->decorationCushions[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < TV_SHOWS_COUNT; ++i) { 
         lu_BitstreamRead_buffer(state, &v->tvShows[i], 36);
      }
   }
   {
      u16 i;
      for (i = 0; i < POKE_NEWS_COUNT; ++i) { 
         lu_BitstreamRead_PokeNews(state, &v->pokeNews[i]);
      }
   }
   v->outbreakPokemonSpecies = lu_BitstreamRead_u16(state, 11) + 0;
   v->outbreakLocationMapNum = lu_BitstreamRead_u8(state, 8);
   v->outbreakLocationMapGroup = lu_BitstreamRead_u8(state, 8);
   v->outbreakPokemonLevel = lu_BitstreamRead_u8(state, 7) + 0;
   v->outbreakUnused1 = lu_BitstreamRead_u8(state, 8);
   v->outbreakUnused2 = lu_BitstreamRead_u8(state, 8);
   {
      u16 i;
      for (i = 0; i < MAX_MON_MOVES; ++i) { 
         v->outbreakPokemonMoves[i] = lu_BitstreamRead_u16(state, 16) + 0;
      }
   }
   v->outbreakUnused3 = lu_BitstreamRead_u8(state, 8);
   v->outbreakPokemonProbability = lu_BitstreamRead_u8(state, 8);
   v->outbreakDaysLeft = lu_BitstreamRead_u16(state, 16);
   lu_BitstreamRead_GabbyAndTyData(state, &v->gabbyAndTyData);
   {
      u16 i;
      for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) { 
         v->easyChatProfile[i] = lu_BitstreamRead_u16(state, 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) { 
         v->easyChatBattleStart[i] = lu_BitstreamRead_u16(state, 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) { 
         v->easyChatBattleWon[i] = lu_BitstreamRead_u16(state, 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) { 
         v->easyChatBattleLost[i] = lu_BitstreamRead_u16(state, 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < MAIL_COUNT; ++i) { 
         lu_BitstreamRead_Mail(state, &v->mail[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < NUM_TRENDY_SAYING_BYTES; ++i) { 
         v->unlockedTrendySayings[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   lu_BitstreamRead_buffer(state, &v->oldMan, 64);
   {
      u16 i;
      for (i = 0; i < SAVED_TRENDS_COUNT; ++i) { 
         lu_BitstreamRead_DewfordTrend(state, &v->dewfordTrends[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < NUM_CONTEST_WINNERS; ++i) { 
         lu_BitstreamRead_ContestWinner(state, &v->contestWinners[i]);
      }
   }
   lu_BitstreamRead_DayCare(state, &v->daycare);
   lu_BitstreamRead_LinkBattleRecords(state, &v->linkBattleRecords);
   {
      u16 i;
      for (i = 0; i < GIFT_RIBBONS_COUNT; ++i) { 
         v->giftRibbons[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   lu_BitstreamRead_ExternalEventData(state, &v->externalEventData);
   lu_BitstreamRead_ExternalEventFlags(state, &v->externalEventFlags);
   lu_BitstreamRead_Roamer(state, &v->roamer);
   lu_BitstreamRead_EnigmaBerry(state, &v->enigmaBerry);
   lu_BitstreamRead_MysteryGiftSave(state, &v->mysteryGift);
   {
      u16 i;
      for (i = 0; i < NUM_TRAINER_HILL_MODES; ++i) { 
         v->trainerHillTimes[i] = lu_BitstreamRead_u32(state, 32);
      }
   }
   lu_BitstreamRead_RamScript(state, &v->ramScript);
   lu_BitstreamRead_RecordMixingGift(state, &v->recordMixingGift);
   {
      u16 i;
      for (i = 0; i < NUM_DEX_FLAG_BYTES; ++i) { 
         v->seen2[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   lu_BitstreamRead_buffer(state, &v->lilycoveLady, 64);
   {
      u16 i;
      for (i = 0; i < 20; ++i) { 
         lu_BitstreamRead_TrainerNameRecord(state, &v->trainerNameRecords[i]);
      }
   }
   {
      u16 i, j;
      for (i = 0; i < UNION_ROOM_KB_ROW_COUNT; ++i) { 
         for (j = 0; j < 21; ++j) { 
            v->registeredTexts[i][j] = lu_BitstreamRead_u8(state, 8);
         }
      }
   }
   lu_BitstreamRead_TrainerHillSave(state, &v->trainerHill);
   lu_BitstreamRead_WaldaPhrase(state, &v->waldaPhrase);
}

void lu_BitstreamWrite_SaveBlock1(struct lu_BitstreamState* state, const struct SaveBlock1* v) {
   lu_BitstreamWrite_Coords16(state, &v->pos);
   lu_BitstreamWrite_WarpData(state, &v->location);
   lu_BitstreamWrite_WarpData(state, &v->continueGameWarp);
   lu_BitstreamWrite_WarpData(state, &v->dynamicWarp);
   lu_BitstreamWrite_WarpData(state, &v->lastHealLocation);
   lu_BitstreamWrite_WarpData(state, &v->escapeWarp);
   lu_BitstreamWrite_u16(state, v->savedMusic, 16);
   lu_BitstreamWrite_u8(state, v->weather, 8);
   lu_BitstreamWrite_u8(state, v->weatherCycleStage, 8);
   lu_BitstreamWrite_u8(state, v->flashLevel, 8);
   lu_BitstreamWrite_u16(state, v->mapLayoutId, 16);
   {
      u16 i;
      for (i = 0; i < 0x100; ++i) { 
         lu_BitstreamWrite_u16(state, v->mapView[i], 16);
      }
   }
   lu_BitstreamWrite_u8(state, v->playerPartyCount, 3);
   {
      u16 i;
      for (i = 0; i < PARTY_SIZE; ++i) { 
         lu_BitstreamWrite_Pokemon(state, &v->playerParty[i]);
      }
   }
   lu_BitstreamWrite_u32(state, v->money, 32);
   lu_BitstreamWrite_u16(state, v->coins, 16);
   lu_BitstreamWrite_u16(state, v->registeredItem, 9);
   {
      u16 i;
      for (i = 0; i < PC_ITEMS_COUNT; ++i) { 
         lu_BitstreamWrite_ItemSlot(state, &v->pcItems[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < BAG_ITEMS_COUNT; ++i) { 
         lu_BitstreamWrite_ItemSlot(state, &v->bagPocket_Items[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < BAG_KEYITEMS_COUNT; ++i) { 
         lu_BitstreamWrite_ItemSlot(state, &v->bagPocket_KeyItems[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < BAG_POKEBALLS_COUNT; ++i) { 
         lu_BitstreamWrite_ItemSlot(state, &v->bagPocket_PokeBalls[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < BAG_TMHM_COUNT; ++i) { 
         lu_BitstreamWrite_ItemSlot(state, &v->bagPocket_TMHM[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < BAG_BERRIES_COUNT; ++i) { 
         lu_BitstreamWrite_ItemSlot(state, &v->bagPocket_Berries[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < POKEBLOCKS_COUNT; ++i) { 
         lu_BitstreamWrite_Pokeblock(state, &v->pokeblocks[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < NUM_DEX_FLAG_BYTES; ++i) { 
         lu_BitstreamWrite_u8(state, v->seen1[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < 3; ++i) { 
         lu_BitstreamWrite_u16(state, v->berryBlenderRecords[i], 16);
      }
   }
   lu_BitstreamWrite_u16(state, v->trainerRematchStepCounter, 16);
   {
      u16 i;
      for (i = 0; i < MAX_REMATCH_ENTRIES; ++i) { 
         lu_BitstreamWrite_u8(state, v->trainerRematches[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < OBJECT_EVENTS_COUNT; ++i) { 
         lu_BitstreamWrite_ObjectEvent(state, &v->objectEvents[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < OBJECT_EVENT_TEMPLATES_COUNT; ++i) { 
         lu_BitstreamWrite_ObjectEventTemplate(state, &v->objectEventTemplates[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < NUM_FLAG_BYTES; ++i) { 
         lu_BitstreamWrite_u8(state, v->flags[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < VARS_COUNT; ++i) { 
         lu_BitstreamWrite_u16(state, v->vars[i], 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < NUM_GAME_STATS; ++i) { 
         lu_BitstreamWrite_u32(state, v->gameStats[i], 32);
      }
   }
   {
      u16 i;
      for (i = 0; i < BERRY_TREES_COUNT; ++i) { 
         lu_BitstreamWrite_BerryTree(state, &v->berryTrees[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < SECRET_BASES_COUNT; ++i) { 
         lu_BitstreamWrite_SecretBase(state, &v->secretBases[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < DECOR_MAX_PLAYERS_HOUSE; ++i) { 
         lu_BitstreamWrite_u8(state, v->playerRoomDecorations[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < DECOR_MAX_PLAYERS_HOUSE; ++i) { 
         lu_BitstreamWrite_u8(state, v->playerRoomDecorationPositions[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < 10; ++i) { 
         lu_BitstreamWrite_u8(state, v->decorationDesks[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < 10; ++i) { 
         lu_BitstreamWrite_u8(state, v->decorationChairs[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < 10; ++i) { 
         lu_BitstreamWrite_u8(state, v->decorationPlants[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < 30; ++i) { 
         lu_BitstreamWrite_u8(state, v->decorationOrnaments[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < 30; ++i) { 
         lu_BitstreamWrite_u8(state, v->decorationMats[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < 10; ++i) { 
         lu_BitstreamWrite_u8(state, v->decorationPosters[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < 40; ++i) { 
         lu_BitstreamWrite_u8(state, v->decorationDolls[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < 10; ++i) { 
         lu_BitstreamWrite_u8(state, v->decorationCushions[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < TV_SHOWS_COUNT; ++i) { 
         lu_BitstreamWrite_buffer(state, &v->tvShows[i], 36);
      }
   }
   {
      u16 i;
      for (i = 0; i < POKE_NEWS_COUNT; ++i) { 
         lu_BitstreamWrite_PokeNews(state, &v->pokeNews[i]);
      }
   }
   lu_BitstreamWrite_u16(state, v->outbreakPokemonSpecies, 11);
   lu_BitstreamWrite_u8(state, v->outbreakLocationMapNum, 8);
   lu_BitstreamWrite_u8(state, v->outbreakLocationMapGroup, 8);
   lu_BitstreamWrite_u8(state, v->outbreakPokemonLevel, 7);
   lu_BitstreamWrite_u8(state, v->outbreakUnused1, 8);
   lu_BitstreamWrite_u8(state, v->outbreakUnused2, 8);
   {
      u16 i;
      for (i = 0; i < MAX_MON_MOVES; ++i) { 
         lu_BitstreamWrite_u16(state, v->outbreakPokemonMoves[i], 16);
      }
   }
   lu_BitstreamWrite_u8(state, v->outbreakUnused3, 8);
   lu_BitstreamWrite_u8(state, v->outbreakPokemonProbability, 8);
   lu_BitstreamWrite_u16(state, v->outbreakDaysLeft, 16);
   lu_BitstreamWrite_GabbyAndTyData(state, &v->gabbyAndTyData);
   {
      u16 i;
      for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) { 
         lu_BitstreamWrite_u16(state, v->easyChatProfile[i], 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) { 
         lu_BitstreamWrite_u16(state, v->easyChatBattleStart[i], 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) { 
         lu_BitstreamWrite_u16(state, v->easyChatBattleWon[i], 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) { 
         lu_BitstreamWrite_u16(state, v->easyChatBattleLost[i], 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < MAIL_COUNT; ++i) { 
         lu_BitstreamWrite_Mail(state, &v->mail[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < NUM_TRENDY_SAYING_BYTES; ++i) { 
         lu_BitstreamWrite_u8(state, v->unlockedTrendySayings[i], 8);
      }
   }
   lu_BitstreamWrite_buffer(state, &v->oldMan, 64);
   {
      u16 i;
      for (i = 0; i < SAVED_TRENDS_COUNT; ++i) { 
         lu_BitstreamWrite_DewfordTrend(state, &v->dewfordTrends[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < NUM_CONTEST_WINNERS; ++i) { 
         lu_BitstreamWrite_ContestWinner(state, &v->contestWinners[i]);
      }
   }
   lu_BitstreamWrite_DayCare(state, &v->daycare);
   lu_BitstreamWrite_LinkBattleRecords(state, &v->linkBattleRecords);
   {
      u16 i;
      for (i = 0; i < GIFT_RIBBONS_COUNT; ++i) { 
         lu_BitstreamWrite_u8(state, v->giftRibbons[i], 8);
      }
   }
   lu_BitstreamWrite_ExternalEventData(state, &v->externalEventData);
   lu_BitstreamWrite_ExternalEventFlags(state, &v->externalEventFlags);
   lu_BitstreamWrite_Roamer(state, &v->roamer);
   lu_BitstreamWrite_EnigmaBerry(state, &v->enigmaBerry);
   lu_BitstreamWrite_MysteryGiftSave(state, &v->mysteryGift);
   {
      u16 i;
      for (i = 0; i < NUM_TRAINER_HILL_MODES; ++i) { 
         lu_BitstreamWrite_u32(state, v->trainerHillTimes[i], 32);
      }
   }
   lu_BitstreamWrite_RamScript(state, &v->ramScript);
   lu_BitstreamWrite_RecordMixingGift(state, &v->recordMixingGift);
   {
      u16 i;
      for (i = 0; i < NUM_DEX_FLAG_BYTES; ++i) { 
         lu_BitstreamWrite_u8(state, v->seen2[i], 8);
      }
   }
   lu_BitstreamWrite_buffer(state, &v->lilycoveLady, 64);
   {
      u16 i;
      for (i = 0; i < 20; ++i) { 
         lu_BitstreamWrite_TrainerNameRecord(state, &v->trainerNameRecords[i]);
      }
   }
   {
      u16 i, j;
      for (i = 0; i < UNION_ROOM_KB_ROW_COUNT; ++i) { 
         for (j = 0; j < 21; ++j) { 
            lu_BitstreamWrite_u8(state, v->registeredTexts[i][j], 8);
         }
      }
   }
   lu_BitstreamWrite_TrainerHillSave(state, &v->trainerHill);
   lu_BitstreamWrite_WaldaPhrase(state, &v->waldaPhrase);
}
