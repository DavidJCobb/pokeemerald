#include "lu/generated/sector-serialize/WorldData.h"

// whole-struct serialize funcs:
#include "lu/generated/struct-serialize/serialize_Coords16.h"
#include "lu/generated/struct-serialize/serialize_WarpData.h"
#include "lu/generated/struct-serialize/serialize_Pokemon.h"
#include "lu/generated/struct-serialize/serialize_ItemSlot.h"
#include "lu/generated/struct-serialize/serialize_Pokeblock.h"
#include "lu/generated/struct-serialize/serialize_ObjectEvent.h"
#include "lu/generated/struct-serialize/serialize_ObjectEventTemplate.h"
#include "lu/generated/struct-serialize/serialize_BerryTree.h"
#include "lu/generated/struct-serialize/serialize_SecretBase.h"
#include "lu/generated/struct-serialize/serialize_PokeNews.h"
#include "lu/generated/struct-serialize/serialize_GabbyAndTyData.h"
#include "lu/generated/struct-serialize/serialize_Mail.h"
#include "lu/generated/struct-serialize/serialize_DewfordTrend.h"
#include "lu/generated/struct-serialize/serialize_ContestWinner.h"
#include "lu/generated/struct-serialize/serialize_DayCare.h"
#include "lu/generated/struct-serialize/serialize_LinkBattleRecords.h"
#include "lu/generated/struct-serialize/serialize_ExternalEventData.h"
#include "lu/generated/struct-serialize/serialize_ExternalEventFlags.h"
#include "lu/generated/struct-serialize/serialize_Roamer.h"
#include "lu/generated/struct-serialize/serialize_EnigmaBerry.h"
#include "lu/generated/struct-serialize/serialize_MysteryGiftSave.h"
#include "lu/generated/struct-serialize/serialize_RamScript.h"
#include "lu/generated/struct-serialize/serialize_RecordMixingGift.h"
#include "lu/generated/struct-serialize/serialize_TrainerNameRecord.h"
#include "lu/generated/struct-serialize/serialize_TrainerHillSave.h"
#include "lu/generated/struct-serialize/serialize_WaldaPhrase.h"

void lu_ReadSaveSector_WorldData00(const u8* src, struct SaveBlock1* p_SaveBlock1) {
   u16 i;
   struct lu_BitstreamState state;
   state.target = (u8*)src;
   state.shift  = 0;
   lu_BitstreamRead_Coords16(&state, &p_SaveBlock1->pos);
   lu_BitstreamRead_WarpData(&state, &p_SaveBlock1->location);
   lu_BitstreamRead_WarpData(&state, &p_SaveBlock1->continueGameWarp);
   lu_BitstreamRead_WarpData(&state, &p_SaveBlock1->dynamicWarp);
   lu_BitstreamRead_WarpData(&state, &p_SaveBlock1->lastHealLocation);
   lu_BitstreamRead_WarpData(&state, &p_SaveBlock1->escapeWarp);
   p_SaveBlock1->savedMusic = lu_BitstreamRead_u16(&state, 16);
   p_SaveBlock1->weather = lu_BitstreamRead_u8(&state, 8);
   p_SaveBlock1->weatherCycleStage = lu_BitstreamRead_u8(&state, 8);
   p_SaveBlock1->flashLevel = lu_BitstreamRead_u8(&state, 8);
   p_SaveBlock1->mapLayoutId = lu_BitstreamRead_u16(&state, 16);
   for (i = 0; i < 0x100; ++i) {
      p_SaveBlock1->mapView[i] = lu_BitstreamRead_u16(&state, 16);
   }
   p_SaveBlock1->playerPartyCount = lu_BitstreamRead_u8(&state, 3) + 0;
   for (i = 0; i < PARTY_SIZE; ++i) {
      lu_BitstreamRead_Pokemon(&state, &p_SaveBlock1->playerParty[i]);
   }
   p_SaveBlock1->money = lu_BitstreamRead_u32(&state, 32);
   p_SaveBlock1->coins = lu_BitstreamRead_u16(&state, 16);
   p_SaveBlock1->registeredItem = lu_BitstreamRead_u16(&state, 9);
   for (i = 0; i < PC_ITEMS_COUNT; ++i) {
      lu_BitstreamRead_ItemSlot(&state, &p_SaveBlock1->pcItems[i]);
   }
   for (i = 0; i < BAG_ITEMS_COUNT; ++i) {
      lu_BitstreamRead_ItemSlot(&state, &p_SaveBlock1->bagPocket_Items[i]);
   }
   for (i = 0; i < BAG_KEYITEMS_COUNT; ++i) {
      lu_BitstreamRead_ItemSlot(&state, &p_SaveBlock1->bagPocket_KeyItems[i]);
   }
   for (i = 0; i < BAG_POKEBALLS_COUNT; ++i) {
      lu_BitstreamRead_ItemSlot(&state, &p_SaveBlock1->bagPocket_PokeBalls[i]);
   }
   for (i = 0; i < BAG_TMHM_COUNT; ++i) {
      lu_BitstreamRead_ItemSlot(&state, &p_SaveBlock1->bagPocket_TMHM[i]);
   }
   for (i = 0; i < BAG_BERRIES_COUNT; ++i) {
      lu_BitstreamRead_ItemSlot(&state, &p_SaveBlock1->bagPocket_Berries[i]);
   }
   for (i = 0; i < POKEBLOCKS_COUNT; ++i) {
      lu_BitstreamRead_Pokeblock(&state, &p_SaveBlock1->pokeblocks[i]);
   }
   for (i = 0; i < NUM_DEX_FLAG_BYTES; ++i) {
      p_SaveBlock1->seen1[i] = lu_BitstreamRead_u8(&state, 8);
   }
   for (i = 0; i < 3; ++i) {
      p_SaveBlock1->berryBlenderRecords[i] = lu_BitstreamRead_u16(&state, 16);
   }
   p_SaveBlock1->trainerRematchStepCounter = lu_BitstreamRead_u16(&state, 16);
   for (i = 0; i < MAX_REMATCH_ENTRIES; ++i) {
      p_SaveBlock1->trainerRematches[i] = lu_BitstreamRead_u8(&state, 8);
   }
   for (i = 0; i < OBJECT_EVENTS_COUNT; ++i) {
      lu_BitstreamRead_ObjectEvent(&state, &p_SaveBlock1->objectEvents[i]);
   }
   for (i = 0; i < OBJECT_EVENT_TEMPLATES_COUNT; ++i) {
      lu_BitstreamRead_ObjectEventTemplate(&state, &p_SaveBlock1->objectEventTemplates[i]);
   }
   for (i = 0; i < NUM_FLAG_BYTES; ++i) {
      p_SaveBlock1->flags[i] = lu_BitstreamRead_u8(&state, 8);
   }
   for (i = 0; i < 13; ++i) {
      p_SaveBlock1->vars[i] = lu_BitstreamRead_u16(&state, 16);
   }
};

void lu_ReadSaveSector_WorldData01(const u8* src, struct SaveBlock1* p_SaveBlock1) {
   u16 i;
   struct lu_BitstreamState state;
   state.target = (u8*)src;
   state.shift  = 0;
   for (i = 13; i < 256; ++i) {
      p_SaveBlock1->vars[i] = lu_BitstreamRead_u16(&state, 16);
   }
   for (i = 0; i < NUM_GAME_STATS; ++i) {
      p_SaveBlock1->gameStats[i] = lu_BitstreamRead_u32(&state, 32);
   }
   for (i = 0; i < BERRY_TREES_COUNT; ++i) {
      lu_BitstreamRead_BerryTree(&state, &p_SaveBlock1->berryTrees[i]);
   }
   for (i = 0; i < 16; ++i) {
      lu_BitstreamRead_SecretBase(&state, &p_SaveBlock1->secretBases[i]);
   }
   p_SaveBlock1->secretBases[16].secretBaseId = lu_BitstreamRead_u8(&state, 8);
   p_SaveBlock1->secretBases[16].toRegister = lu_BitstreamRead_u8(&state, 4);
   p_SaveBlock1->secretBases[16].gender = lu_BitstreamRead_bool(&state);
   p_SaveBlock1->secretBases[16].battledOwnerToday = lu_BitstreamRead_bool(&state);
   p_SaveBlock1->secretBases[16].registryStatus = lu_BitstreamRead_u8(&state, 2);
   lu_BitstreamRead_string_optional_terminator(&state, p_SaveBlock1->secretBases[16].trainerName, PLAYER_NAME_LENGTH);
   for (i = 0; i < TRAINER_ID_LENGTH; ++i) {
      p_SaveBlock1->secretBases[16].trainerId[i] = lu_BitstreamRead_u8(&state, 8);
   }
   p_SaveBlock1->secretBases[16].language = lu_BitstreamRead_u8(&state, 3) + 0;
   p_SaveBlock1->secretBases[16].numSecretBasesReceived = lu_BitstreamRead_u16(&state, 16);
   p_SaveBlock1->secretBases[16].numTimesEntered = lu_BitstreamRead_u8(&state, 8);
   p_SaveBlock1->secretBases[16].unused = lu_BitstreamRead_u8(&state, 8);
   for (i = 0; i < DECOR_MAX_SECRET_BASE; ++i) {
      p_SaveBlock1->secretBases[16].decorations[i] = lu_BitstreamRead_u8(&state, 8);
   }
   for (i = 0; i < DECOR_MAX_SECRET_BASE; ++i) {
      p_SaveBlock1->secretBases[16].decorationPositions[i] = lu_BitstreamRead_u8(&state, 8);
   }
   for (i = 0; i < PARTY_SIZE; ++i) {
      p_SaveBlock1->secretBases[16].party.personality[i] = lu_BitstreamRead_u32(&state, 32);
   }
   for (i = 0; i < 10; ++i) {
      p_SaveBlock1->secretBases[16].party.moves[i] = lu_BitstreamRead_u16(&state, 16) + 0;
   }
};

void lu_ReadSaveSector_WorldData02(const u8* src, struct SaveBlock1* p_SaveBlock1) {
   u8 i;
   struct lu_BitstreamState state;
   state.target = (u8*)src;
   state.shift  = 0;
   for (i = 10; i < 24; ++i) {
      p_SaveBlock1->secretBases[16].party.moves[i] = lu_BitstreamRead_u16(&state, 16) + 0;
   }
   for (i = 0; i < PARTY_SIZE; ++i) {
      p_SaveBlock1->secretBases[16].party.species[i] = lu_BitstreamRead_u16(&state, 11) + 0;
   }
   for (i = 0; i < PARTY_SIZE; ++i) {
      p_SaveBlock1->secretBases[16].party.heldItems[i] = lu_BitstreamRead_u16(&state, 9);
   }
   for (i = 0; i < PARTY_SIZE; ++i) {
      p_SaveBlock1->secretBases[16].party.levels[i] = lu_BitstreamRead_u8(&state, 7) + 0;
   }
   for (i = 0; i < PARTY_SIZE; ++i) {
      p_SaveBlock1->secretBases[16].party.EVs[i] = lu_BitstreamRead_u8(&state, 8);
   }
   for (i = 17; i < 20; ++i) {
      lu_BitstreamRead_SecretBase(&state, &p_SaveBlock1->secretBases[i]);
   }
   for (i = 0; i < DECOR_MAX_PLAYERS_HOUSE; ++i) {
      p_SaveBlock1->playerRoomDecorations[i] = lu_BitstreamRead_u8(&state, 8);
   }
   for (i = 0; i < DECOR_MAX_PLAYERS_HOUSE; ++i) {
      p_SaveBlock1->playerRoomDecorationPositions[i] = lu_BitstreamRead_u8(&state, 8);
   }
   for (i = 0; i < 10; ++i) {
      p_SaveBlock1->decorationDesks[i] = lu_BitstreamRead_u8(&state, 8);
   }
   for (i = 0; i < 10; ++i) {
      p_SaveBlock1->decorationChairs[i] = lu_BitstreamRead_u8(&state, 8);
   }
   for (i = 0; i < 10; ++i) {
      p_SaveBlock1->decorationPlants[i] = lu_BitstreamRead_u8(&state, 8);
   }
   for (i = 0; i < 30; ++i) {
      p_SaveBlock1->decorationOrnaments[i] = lu_BitstreamRead_u8(&state, 8);
   }
   for (i = 0; i < 30; ++i) {
      p_SaveBlock1->decorationMats[i] = lu_BitstreamRead_u8(&state, 8);
   }
   for (i = 0; i < 10; ++i) {
      p_SaveBlock1->decorationPosters[i] = lu_BitstreamRead_u8(&state, 8);
   }
   for (i = 0; i < 40; ++i) {
      p_SaveBlock1->decorationDolls[i] = lu_BitstreamRead_u8(&state, 8);
   }
   for (i = 0; i < 10; ++i) {
      p_SaveBlock1->decorationCushions[i] = lu_BitstreamRead_u8(&state, 8);
   }
   for (i = 0; i < TV_SHOWS_COUNT; ++i) {
      lu_BitstreamRead_buffer(&state, &p_SaveBlock1->tvShows[i], 36);
   }
   for (i = 0; i < POKE_NEWS_COUNT; ++i) {
      lu_BitstreamRead_PokeNews(&state, &p_SaveBlock1->pokeNews[i]);
   }
   p_SaveBlock1->outbreakPokemonSpecies = lu_BitstreamRead_u16(&state, 11) + 0;
   p_SaveBlock1->outbreakLocationMapNum = lu_BitstreamRead_u8(&state, 8);
   p_SaveBlock1->outbreakLocationMapGroup = lu_BitstreamRead_u8(&state, 8);
   p_SaveBlock1->outbreakPokemonLevel = lu_BitstreamRead_u8(&state, 7) + 0;
   p_SaveBlock1->outbreakUnused1 = lu_BitstreamRead_u8(&state, 8);
   p_SaveBlock1->outbreakUnused2 = lu_BitstreamRead_u8(&state, 8);
   for (i = 0; i < MAX_MON_MOVES; ++i) {
      p_SaveBlock1->outbreakPokemonMoves[i] = lu_BitstreamRead_u16(&state, 16) + 0;
   }
   p_SaveBlock1->outbreakUnused3 = lu_BitstreamRead_u8(&state, 8);
   p_SaveBlock1->outbreakPokemonProbability = lu_BitstreamRead_u8(&state, 8);
   p_SaveBlock1->outbreakDaysLeft = lu_BitstreamRead_u16(&state, 16);
   lu_BitstreamRead_GabbyAndTyData(&state, &p_SaveBlock1->gabbyAndTyData);
   for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) {
      p_SaveBlock1->easyChatProfile[i] = lu_BitstreamRead_u16(&state, 16);
   }
   for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) {
      p_SaveBlock1->easyChatBattleStart[i] = lu_BitstreamRead_u16(&state, 16);
   }
   for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) {
      p_SaveBlock1->easyChatBattleWon[i] = lu_BitstreamRead_u16(&state, 16);
   }
   for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) {
      p_SaveBlock1->easyChatBattleLost[i] = lu_BitstreamRead_u16(&state, 16);
   }
   for (i = 0; i < MAIL_COUNT; ++i) {
      lu_BitstreamRead_Mail(&state, &p_SaveBlock1->mail[i]);
   }
   for (i = 0; i < NUM_TRENDY_SAYING_BYTES; ++i) {
      p_SaveBlock1->unlockedTrendySayings[i] = lu_BitstreamRead_u8(&state, 8);
   }
   lu_BitstreamRead_buffer(&state, &p_SaveBlock1->oldMan, 64);
   for (i = 0; i < SAVED_TRENDS_COUNT; ++i) {
      lu_BitstreamRead_DewfordTrend(&state, &p_SaveBlock1->dewfordTrends[i]);
   }
   for (i = 0; i < NUM_CONTEST_WINNERS; ++i) {
      lu_BitstreamRead_ContestWinner(&state, &p_SaveBlock1->contestWinners[i]);
   }
   lu_BitstreamRead_DayCare(&state, &p_SaveBlock1->daycare);
   lu_BitstreamRead_LinkBattleRecords(&state, &p_SaveBlock1->linkBattleRecords);
   for (i = 0; i < GIFT_RIBBONS_COUNT; ++i) {
      p_SaveBlock1->giftRibbons[i] = lu_BitstreamRead_u8(&state, 8);
   }
   lu_BitstreamRead_ExternalEventData(&state, &p_SaveBlock1->externalEventData);
   lu_BitstreamRead_ExternalEventFlags(&state, &p_SaveBlock1->externalEventFlags);
   lu_BitstreamRead_Roamer(&state, &p_SaveBlock1->roamer);
   lu_BitstreamRead_EnigmaBerry(&state, &p_SaveBlock1->enigmaBerry);
   lu_BitstreamRead_MysteryGiftSave(&state, &p_SaveBlock1->mysteryGift);
};

void lu_ReadSaveSector_WorldData03(const u8* src, struct SaveBlock1* p_SaveBlock1) {
   u8 i, j;
   struct lu_BitstreamState state;
   state.target = (u8*)src;
   state.shift  = 0;
   for (i = 0; i < 4; ++i) {
      p_SaveBlock1->trainerHillTimes[i] = lu_BitstreamRead_u32(&state, 32);
   }
   lu_BitstreamRead_RamScript(&state, &p_SaveBlock1->ramScript);
   lu_BitstreamRead_RecordMixingGift(&state, &p_SaveBlock1->recordMixingGift);
   for (i = 0; i < NUM_DEX_FLAG_BYTES; ++i) {
      p_SaveBlock1->seen2[i] = lu_BitstreamRead_u8(&state, 8);
   }
   lu_BitstreamRead_buffer(&state, &p_SaveBlock1->lilycoveLady, 64);
   for (i = 0; i < 20; ++i) {
      lu_BitstreamRead_TrainerNameRecord(&state, &p_SaveBlock1->trainerNameRecords[i]);
   }
   for (i = 0; i < UNION_ROOM_KB_ROW_COUNT; ++i) {
      for (j = 0; j < 21; ++j) {
         p_SaveBlock1->registeredTexts[i][j] = lu_BitstreamRead_u8(&state, 8);
      }
   }
   lu_BitstreamRead_TrainerHillSave(&state, &p_SaveBlock1->trainerHill);
   lu_BitstreamRead_WaldaPhrase(&state, &p_SaveBlock1->waldaPhrase);
};

void lu_WriteSaveSector_WorldData00(u8* dst, const struct SaveBlock1* p_SaveBlock1) {
   u16 i;
   struct lu_BitstreamState state;
   state.target = dst;
   state.shift  = 0;
   lu_BitstreamWrite_Coords16(&state, &p_SaveBlock1->pos);
   lu_BitstreamWrite_WarpData(&state, &p_SaveBlock1->location);
   lu_BitstreamWrite_WarpData(&state, &p_SaveBlock1->continueGameWarp);
   lu_BitstreamWrite_WarpData(&state, &p_SaveBlock1->dynamicWarp);
   lu_BitstreamWrite_WarpData(&state, &p_SaveBlock1->lastHealLocation);
   lu_BitstreamWrite_WarpData(&state, &p_SaveBlock1->escapeWarp);
   lu_BitstreamWrite_u16(&state, p_SaveBlock1->savedMusic, 16);
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->weather, 8);
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->weatherCycleStage, 8);
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->flashLevel, 8);
   lu_BitstreamWrite_u16(&state, p_SaveBlock1->mapLayoutId, 16);
   for (i = 0; i < 0x100; ++i) {
      lu_BitstreamWrite_u16(&state, p_SaveBlock1->mapView[i], 16);
   }
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->playerPartyCount - 0, 3);
   for (i = 0; i < PARTY_SIZE; ++i) {
      lu_BitstreamWrite_Pokemon(&state, &p_SaveBlock1->playerParty[i]);
   }
   lu_BitstreamWrite_u32(&state, p_SaveBlock1->money, 32);
   lu_BitstreamWrite_u16(&state, p_SaveBlock1->coins, 16);
   lu_BitstreamWrite_u16(&state, p_SaveBlock1->registeredItem, 9);
   for (i = 0; i < PC_ITEMS_COUNT; ++i) {
      lu_BitstreamWrite_ItemSlot(&state, &p_SaveBlock1->pcItems[i]);
   }
   for (i = 0; i < BAG_ITEMS_COUNT; ++i) {
      lu_BitstreamWrite_ItemSlot(&state, &p_SaveBlock1->bagPocket_Items[i]);
   }
   for (i = 0; i < BAG_KEYITEMS_COUNT; ++i) {
      lu_BitstreamWrite_ItemSlot(&state, &p_SaveBlock1->bagPocket_KeyItems[i]);
   }
   for (i = 0; i < BAG_POKEBALLS_COUNT; ++i) {
      lu_BitstreamWrite_ItemSlot(&state, &p_SaveBlock1->bagPocket_PokeBalls[i]);
   }
   for (i = 0; i < BAG_TMHM_COUNT; ++i) {
      lu_BitstreamWrite_ItemSlot(&state, &p_SaveBlock1->bagPocket_TMHM[i]);
   }
   for (i = 0; i < BAG_BERRIES_COUNT; ++i) {
      lu_BitstreamWrite_ItemSlot(&state, &p_SaveBlock1->bagPocket_Berries[i]);
   }
   for (i = 0; i < POKEBLOCKS_COUNT; ++i) {
      lu_BitstreamWrite_Pokeblock(&state, &p_SaveBlock1->pokeblocks[i]);
   }
   for (i = 0; i < NUM_DEX_FLAG_BYTES; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->seen1[i], 8);
   }
   for (i = 0; i < 3; ++i) {
      lu_BitstreamWrite_u16(&state, p_SaveBlock1->berryBlenderRecords[i], 16);
   }
   lu_BitstreamWrite_u16(&state, p_SaveBlock1->trainerRematchStepCounter, 16);
   for (i = 0; i < MAX_REMATCH_ENTRIES; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->trainerRematches[i], 8);
   }
   for (i = 0; i < OBJECT_EVENTS_COUNT; ++i) {
      lu_BitstreamWrite_ObjectEvent(&state, &p_SaveBlock1->objectEvents[i]);
   }
   for (i = 0; i < OBJECT_EVENT_TEMPLATES_COUNT; ++i) {
      lu_BitstreamWrite_ObjectEventTemplate(&state, &p_SaveBlock1->objectEventTemplates[i]);
   }
   for (i = 0; i < NUM_FLAG_BYTES; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->flags[i], 8);
   }
   for (i = 0; i < 13; ++i) {
      lu_BitstreamWrite_u16(&state, p_SaveBlock1->vars[i], 16);
   }
};

void lu_WriteSaveSector_WorldData01(u8* dst, const struct SaveBlock1* p_SaveBlock1) {
   u16 i;
   struct lu_BitstreamState state;
   state.target = dst;
   state.shift  = 0;
   for (i = 13; i < 256; ++i) {
      lu_BitstreamWrite_u16(&state, p_SaveBlock1->vars[i], 16);
   }
   for (i = 0; i < NUM_GAME_STATS; ++i) {
      lu_BitstreamWrite_u32(&state, p_SaveBlock1->gameStats[i], 32);
   }
   for (i = 0; i < BERRY_TREES_COUNT; ++i) {
      lu_BitstreamWrite_BerryTree(&state, &p_SaveBlock1->berryTrees[i]);
   }
   for (i = 0; i < 16; ++i) {
      lu_BitstreamWrite_SecretBase(&state, &p_SaveBlock1->secretBases[i]);
   }
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->secretBases[16].secretBaseId, 8);
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->secretBases[16].toRegister, 4);
   lu_BitstreamWrite_bool(&state, p_SaveBlock1->secretBases[16].gender);
   lu_BitstreamWrite_bool(&state, p_SaveBlock1->secretBases[16].battledOwnerToday);
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->secretBases[16].registryStatus, 2);
   lu_BitstreamWrite_string_optional_terminator(&state, p_SaveBlock1->secretBases[16].trainerName, PLAYER_NAME_LENGTH);
   for (i = 0; i < TRAINER_ID_LENGTH; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->secretBases[16].trainerId[i], 8);
   }
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->secretBases[16].language - 0, 3);
   lu_BitstreamWrite_u16(&state, p_SaveBlock1->secretBases[16].numSecretBasesReceived, 16);
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->secretBases[16].numTimesEntered, 8);
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->secretBases[16].unused, 8);
   for (i = 0; i < DECOR_MAX_SECRET_BASE; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->secretBases[16].decorations[i], 8);
   }
   for (i = 0; i < DECOR_MAX_SECRET_BASE; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->secretBases[16].decorationPositions[i], 8);
   }
   for (i = 0; i < PARTY_SIZE; ++i) {
      lu_BitstreamWrite_u32(&state, p_SaveBlock1->secretBases[16].party.personality[i], 32);
   }
   for (i = 0; i < 10; ++i) {
      lu_BitstreamWrite_u16(&state, p_SaveBlock1->secretBases[16].party.moves[i] - 0, 16);
   }
};

void lu_WriteSaveSector_WorldData02(u8* dst, const struct SaveBlock1* p_SaveBlock1) {
   u8 i;
   struct lu_BitstreamState state;
   state.target = dst;
   state.shift  = 0;
   for (i = 10; i < 24; ++i) {
      lu_BitstreamWrite_u16(&state, p_SaveBlock1->secretBases[16].party.moves[i] - 0, 16);
   }
   for (i = 0; i < PARTY_SIZE; ++i) {
      lu_BitstreamWrite_u16(&state, p_SaveBlock1->secretBases[16].party.species[i] - 0, 11);
   }
   for (i = 0; i < PARTY_SIZE; ++i) {
      lu_BitstreamWrite_u16(&state, p_SaveBlock1->secretBases[16].party.heldItems[i], 9);
   }
   for (i = 0; i < PARTY_SIZE; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->secretBases[16].party.levels[i] - 0, 7);
   }
   for (i = 0; i < PARTY_SIZE; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->secretBases[16].party.EVs[i], 8);
   }
   for (i = 17; i < 20; ++i) {
      lu_BitstreamWrite_SecretBase(&state, &p_SaveBlock1->secretBases[i]);
   }
   for (i = 0; i < DECOR_MAX_PLAYERS_HOUSE; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->playerRoomDecorations[i], 8);
   }
   for (i = 0; i < DECOR_MAX_PLAYERS_HOUSE; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->playerRoomDecorationPositions[i], 8);
   }
   for (i = 0; i < 10; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->decorationDesks[i], 8);
   }
   for (i = 0; i < 10; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->decorationChairs[i], 8);
   }
   for (i = 0; i < 10; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->decorationPlants[i], 8);
   }
   for (i = 0; i < 30; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->decorationOrnaments[i], 8);
   }
   for (i = 0; i < 30; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->decorationMats[i], 8);
   }
   for (i = 0; i < 10; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->decorationPosters[i], 8);
   }
   for (i = 0; i < 40; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->decorationDolls[i], 8);
   }
   for (i = 0; i < 10; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->decorationCushions[i], 8);
   }
   for (i = 0; i < TV_SHOWS_COUNT; ++i) {
      lu_BitstreamWrite_buffer(&state, &p_SaveBlock1->tvShows[i], 36);
   }
   for (i = 0; i < POKE_NEWS_COUNT; ++i) {
      lu_BitstreamWrite_PokeNews(&state, &p_SaveBlock1->pokeNews[i]);
   }
   lu_BitstreamWrite_u16(&state, p_SaveBlock1->outbreakPokemonSpecies - 0, 11);
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->outbreakLocationMapNum, 8);
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->outbreakLocationMapGroup, 8);
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->outbreakPokemonLevel - 0, 7);
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->outbreakUnused1, 8);
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->outbreakUnused2, 8);
   for (i = 0; i < MAX_MON_MOVES; ++i) {
      lu_BitstreamWrite_u16(&state, p_SaveBlock1->outbreakPokemonMoves[i] - 0, 16);
   }
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->outbreakUnused3, 8);
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->outbreakPokemonProbability, 8);
   lu_BitstreamWrite_u16(&state, p_SaveBlock1->outbreakDaysLeft, 16);
   lu_BitstreamWrite_GabbyAndTyData(&state, &p_SaveBlock1->gabbyAndTyData);
   for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) {
      lu_BitstreamWrite_u16(&state, p_SaveBlock1->easyChatProfile[i], 16);
   }
   for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) {
      lu_BitstreamWrite_u16(&state, p_SaveBlock1->easyChatBattleStart[i], 16);
   }
   for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) {
      lu_BitstreamWrite_u16(&state, p_SaveBlock1->easyChatBattleWon[i], 16);
   }
   for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) {
      lu_BitstreamWrite_u16(&state, p_SaveBlock1->easyChatBattleLost[i], 16);
   }
   for (i = 0; i < MAIL_COUNT; ++i) {
      lu_BitstreamWrite_Mail(&state, &p_SaveBlock1->mail[i]);
   }
   for (i = 0; i < NUM_TRENDY_SAYING_BYTES; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->unlockedTrendySayings[i], 8);
   }
   lu_BitstreamWrite_buffer(&state, &p_SaveBlock1->oldMan, 64);
   for (i = 0; i < SAVED_TRENDS_COUNT; ++i) {
      lu_BitstreamWrite_DewfordTrend(&state, &p_SaveBlock1->dewfordTrends[i]);
   }
   for (i = 0; i < NUM_CONTEST_WINNERS; ++i) {
      lu_BitstreamWrite_ContestWinner(&state, &p_SaveBlock1->contestWinners[i]);
   }
   lu_BitstreamWrite_DayCare(&state, &p_SaveBlock1->daycare);
   lu_BitstreamWrite_LinkBattleRecords(&state, &p_SaveBlock1->linkBattleRecords);
   for (i = 0; i < GIFT_RIBBONS_COUNT; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->giftRibbons[i], 8);
   }
   lu_BitstreamWrite_ExternalEventData(&state, &p_SaveBlock1->externalEventData);
   lu_BitstreamWrite_ExternalEventFlags(&state, &p_SaveBlock1->externalEventFlags);
   lu_BitstreamWrite_Roamer(&state, &p_SaveBlock1->roamer);
   lu_BitstreamWrite_EnigmaBerry(&state, &p_SaveBlock1->enigmaBerry);
   lu_BitstreamWrite_MysteryGiftSave(&state, &p_SaveBlock1->mysteryGift);
};

void lu_WriteSaveSector_WorldData03(u8* dst, const struct SaveBlock1* p_SaveBlock1) {
   u8 i, j;
   struct lu_BitstreamState state;
   state.target = dst;
   state.shift  = 0;
   for (i = 0; i < 4; ++i) {
      lu_BitstreamWrite_u32(&state, p_SaveBlock1->trainerHillTimes[i], 32);
   }
   lu_BitstreamWrite_RamScript(&state, &p_SaveBlock1->ramScript);
   lu_BitstreamWrite_RecordMixingGift(&state, &p_SaveBlock1->recordMixingGift);
   for (i = 0; i < NUM_DEX_FLAG_BYTES; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->seen2[i], 8);
   }
   lu_BitstreamWrite_buffer(&state, &p_SaveBlock1->lilycoveLady, 64);
   for (i = 0; i < 20; ++i) {
      lu_BitstreamWrite_TrainerNameRecord(&state, &p_SaveBlock1->trainerNameRecords[i]);
   }
   for (i = 0; i < UNION_ROOM_KB_ROW_COUNT; ++i) {
      for (j = 0; j < 21; ++j) {
         lu_BitstreamWrite_u8(&state, p_SaveBlock1->registeredTexts[i][j], 8);
      }
   }
   lu_BitstreamWrite_TrainerHillSave(&state, &p_SaveBlock1->trainerHill);
   lu_BitstreamWrite_WaldaPhrase(&state, &p_SaveBlock1->waldaPhrase);
};

