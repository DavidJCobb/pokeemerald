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
#include "lu/generated/struct-serialize/serialize_WonderNews.h"
#include "lu/generated/struct-serialize/serialize_WonderCardMetadata.h"
#include "lu/generated/struct-serialize/serialize_WonderNewsMetadata.h"
#include "lu/generated/struct-serialize/serialize_RamScript.h"
#include "lu/generated/struct-serialize/serialize_RecordMixingGift.h"
#include "lu/generated/struct-serialize/serialize_TrainerNameRecord.h"
#include "lu/generated/struct-serialize/serialize_TrainerHillSave.h"
#include "lu/generated/struct-serialize/serialize_WaldaPhrase.h"

void lu_ReadSaveSector_WorldData00(const u8* src, struct SaveBlock1* p_SaveBlock1) {
   u16 i;
   struct lu_BitstreamState state;
   lu_BitstreamInitialize(&state, (u8*)src); // need to cast away constness to store it here

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
   p_SaveBlock1->playerPartyCount = lu_BitstreamRead_u8(&state, 3);
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
   for (i = 0; i < 62; ++i) {
      p_SaveBlock1->flags[i] = lu_BitstreamRead_u8(&state, 8);
   }
};

void lu_ReadSaveSector_WorldData01(const u8* src, struct SaveBlock1* p_SaveBlock1) {
   u16 i;
   struct lu_BitstreamState state;
   lu_BitstreamInitialize(&state, (u8*)src); // need to cast away constness to store it here

   for (i = 0; i < 300; ++i) {
      p_SaveBlock1->flags[i] = lu_BitstreamRead_u8(&state, 8);
   }
   for (i = 0; i < VARS_COUNT; ++i) {
      p_SaveBlock1->vars[i] = lu_BitstreamRead_u16(&state, 16);
   }
   for (i = 0; i < NUM_GAME_STATS; ++i) {
      p_SaveBlock1->gameStats[i] = lu_BitstreamRead_u32(&state, 32);
   }
   for (i = 0; i < BERRY_TREES_COUNT; ++i) {
      lu_BitstreamRead_BerryTree(&state, &p_SaveBlock1->berryTrees[i]);
   }
   for (i = 0; i < 14; ++i) {
      lu_BitstreamRead_SecretBase(&state, &p_SaveBlock1->secretBases[i]);
   }
   p_SaveBlock1->secretBases[14].secretBaseId = lu_BitstreamRead_u8(&state, 8);
   p_SaveBlock1->secretBases[14].toRegister = lu_BitstreamRead_u8(&state, 4);
   p_SaveBlock1->secretBases[14].gender = lu_BitstreamRead_bool(&state);
   p_SaveBlock1->secretBases[14].battledOwnerToday = lu_BitstreamRead_bool(&state);
   p_SaveBlock1->secretBases[14].registryStatus = lu_BitstreamRead_u8(&state, 2);
   lu_BitstreamRead_string_optional_terminator(&state, p_SaveBlock1->secretBases[14].trainerName, PLAYER_NAME_LENGTH);
   for (i = 0; i < TRAINER_ID_LENGTH; ++i) {
      p_SaveBlock1->secretBases[14].trainerId[i] = lu_BitstreamRead_u8(&state, 8);
   }
   p_SaveBlock1->secretBases[14].language = lu_BitstreamRead_u8(&state, 3);
   p_SaveBlock1->secretBases[14].numSecretBasesReceived = lu_BitstreamRead_u16(&state, 16);
   p_SaveBlock1->secretBases[14].numTimesEntered = lu_BitstreamRead_u8(&state, 8);
   p_SaveBlock1->secretBases[14].unused = lu_BitstreamRead_u8(&state, 8);
   for (i = 0; i < DECOR_MAX_SECRET_BASE; ++i) {
      p_SaveBlock1->secretBases[14].decorations[i] = lu_BitstreamRead_u8(&state, 8);
   }
   for (i = 0; i < DECOR_MAX_SECRET_BASE; ++i) {
      p_SaveBlock1->secretBases[14].decorationPositions[i] = lu_BitstreamRead_u8(&state, 8);
   }
   for (i = 0; i < PARTY_SIZE; ++i) {
      p_SaveBlock1->secretBases[14].party.personality[i] = lu_BitstreamRead_u32(&state, 32);
   }
   for (i = 0; i < PARTY_SIZE * MAX_MON_MOVES; ++i) {
      p_SaveBlock1->secretBases[14].party.moves[i] = lu_BitstreamRead_u16(&state, 16);
   }
   for (i = 0; i < 3; ++i) {
      p_SaveBlock1->secretBases[14].party.species[i] = lu_BitstreamRead_u16(&state, 11);
   }
};

void lu_ReadSaveSector_WorldData02(const u8* src, struct SaveBlock1* p_SaveBlock1) {
   u8 i;
   struct lu_BitstreamState state;
   lu_BitstreamInitialize(&state, (u8*)src); // need to cast away constness to store it here

   for (i = 0; i < 6; ++i) {
      p_SaveBlock1->secretBases[14].party.species[i] = lu_BitstreamRead_u16(&state, 11);
   }
   for (i = 0; i < PARTY_SIZE; ++i) {
      p_SaveBlock1->secretBases[14].party.heldItems[i] = lu_BitstreamRead_u16(&state, 9);
   }
   for (i = 0; i < PARTY_SIZE; ++i) {
      p_SaveBlock1->secretBases[14].party.levels[i] = lu_BitstreamRead_u8(&state, 7);
   }
   for (i = 0; i < PARTY_SIZE; ++i) {
      p_SaveBlock1->secretBases[14].party.EVs[i] = lu_BitstreamRead_u8(&state, 8);
   }
   for (i = 0; i < 20; ++i) {
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
   p_SaveBlock1->outbreakPokemonSpecies = lu_BitstreamRead_u16(&state, 11);
   p_SaveBlock1->outbreakLocationMapNum = lu_BitstreamRead_u8(&state, 8);
   p_SaveBlock1->outbreakLocationMapGroup = lu_BitstreamRead_u8(&state, 8);
   p_SaveBlock1->outbreakPokemonLevel = lu_BitstreamRead_u8(&state, 7);
   p_SaveBlock1->outbreakUnused1 = lu_BitstreamRead_u8(&state, 8);
   p_SaveBlock1->outbreakUnused2 = lu_BitstreamRead_u8(&state, 8);
   for (i = 0; i < MAX_MON_MOVES; ++i) {
      p_SaveBlock1->outbreakPokemonMoves[i] = lu_BitstreamRead_u16(&state, 16);
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
   p_SaveBlock1->mysteryGift.newsCrc = lu_BitstreamRead_u32(&state, 32);
   lu_BitstreamRead_WonderNews(&state, &p_SaveBlock1->mysteryGift.news);
   p_SaveBlock1->mysteryGift.cardCrc = lu_BitstreamRead_u32(&state, 32);
   p_SaveBlock1->mysteryGift.card.flagId = lu_BitstreamRead_u16(&state, 16);
   p_SaveBlock1->mysteryGift.card.iconSpecies = lu_BitstreamRead_u16(&state, 11);
   p_SaveBlock1->mysteryGift.card.idNumber = lu_BitstreamRead_u32(&state, 32);
   p_SaveBlock1->mysteryGift.card.type = lu_BitstreamRead_u8(&state, 2);
   p_SaveBlock1->mysteryGift.card.bgType = lu_BitstreamRead_u8(&state, 4);
   p_SaveBlock1->mysteryGift.card.sendType = lu_BitstreamRead_u8(&state, 2);
   p_SaveBlock1->mysteryGift.card.maxStamps = lu_BitstreamRead_u8(&state, 8);
   lu_BitstreamRead_string_optional_terminator(&state, p_SaveBlock1->mysteryGift.card.titleText, WONDER_CARD_TEXT_LENGTH);
   lu_BitstreamRead_string_optional_terminator(&state, p_SaveBlock1->mysteryGift.card.subtitleText, WONDER_CARD_TEXT_LENGTH);
};

void lu_ReadSaveSector_WorldData03(const u8* src, struct SaveBlock1* p_SaveBlock1) {
   u8 i, j;
   struct lu_BitstreamState state;
   lu_BitstreamInitialize(&state, (u8*)src); // need to cast away constness to store it here

   for (i = 0; i < 4; ++i) {
      lu_BitstreamRead_string_optional_terminator(&state, p_SaveBlock1->mysteryGift.card.bodyText[i], WONDER_CARD_TEXT_LENGTH);
   }
   lu_BitstreamRead_string_optional_terminator(&state, p_SaveBlock1->mysteryGift.card.footerLine1Text, WONDER_CARD_TEXT_LENGTH);
   lu_BitstreamRead_string_optional_terminator(&state, p_SaveBlock1->mysteryGift.card.footerLine2Text, WONDER_CARD_TEXT_LENGTH);
   p_SaveBlock1->mysteryGift.cardMetadataCrc = lu_BitstreamRead_u32(&state, 32);
   lu_BitstreamRead_WonderCardMetadata(&state, &p_SaveBlock1->mysteryGift.cardMetadata);
   for (i = 0; i < NUM_QUESTIONNAIRE_WORDS; ++i) {
      p_SaveBlock1->mysteryGift.questionnaireWords[i] = lu_BitstreamRead_u16(&state, 16);
   }
   lu_BitstreamRead_WonderNewsMetadata(&state, &p_SaveBlock1->mysteryGift.newsMetadata);
   for (i = 0; i < 2; ++i) {
      for (j = 0; j < 5; ++j) {
         p_SaveBlock1->mysteryGift.trainerIds[i][j] = lu_BitstreamRead_u32(&state, 32);
      }
   }
   for (i = 0; i < NUM_TRAINER_HILL_MODES; ++i) {
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
   lu_BitstreamInitialize(&state, dst);

   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->pos", 0);
   #endif
   lu_BitstreamWrite_Coords16(&state, &p_SaveBlock1->pos);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->location", 0);
   #endif
   lu_BitstreamWrite_WarpData(&state, &p_SaveBlock1->location);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->continueGameWarp", 0);
   #endif
   lu_BitstreamWrite_WarpData(&state, &p_SaveBlock1->continueGameWarp);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->dynamicWarp", 0);
   #endif
   lu_BitstreamWrite_WarpData(&state, &p_SaveBlock1->dynamicWarp);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->lastHealLocation", 0);
   #endif
   lu_BitstreamWrite_WarpData(&state, &p_SaveBlock1->lastHealLocation);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->escapeWarp", 0);
   #endif
   lu_BitstreamWrite_WarpData(&state, &p_SaveBlock1->escapeWarp);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->savedMusic", 0);
   #endif
   lu_BitstreamWrite_u16(&state, p_SaveBlock1->savedMusic, 16);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->weather", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->weather, 8);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->weatherCycleStage", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->weatherCycleStage, 8);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->flashLevel", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->flashLevel, 8);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->mapLayoutId", 0);
   #endif
   lu_BitstreamWrite_u16(&state, p_SaveBlock1->mapLayoutId, 16);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->mapView", 0);
   #endif
   for (i = 0; i < 0x100; ++i) {
      lu_BitstreamWrite_u16(&state, p_SaveBlock1->mapView[i], 16);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->playerPartyCount", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->playerPartyCount, 3);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->playerParty", 0);
   #endif
   for (i = 0; i < PARTY_SIZE; ++i) {
      lu_BitstreamWrite_Pokemon(&state, &p_SaveBlock1->playerParty[i]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->money", 0);
   #endif
   lu_BitstreamWrite_u32(&state, p_SaveBlock1->money, 32);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->coins", 0);
   #endif
   lu_BitstreamWrite_u16(&state, p_SaveBlock1->coins, 16);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->registeredItem", 0);
   #endif
   lu_BitstreamWrite_u16(&state, p_SaveBlock1->registeredItem, 9);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->pcItems", 0);
   #endif
   for (i = 0; i < PC_ITEMS_COUNT; ++i) {
      lu_BitstreamWrite_ItemSlot(&state, &p_SaveBlock1->pcItems[i]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->bagPocket_Items", 0);
   #endif
   for (i = 0; i < BAG_ITEMS_COUNT; ++i) {
      lu_BitstreamWrite_ItemSlot(&state, &p_SaveBlock1->bagPocket_Items[i]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->bagPocket_KeyItems", 0);
   #endif
   for (i = 0; i < BAG_KEYITEMS_COUNT; ++i) {
      lu_BitstreamWrite_ItemSlot(&state, &p_SaveBlock1->bagPocket_KeyItems[i]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->bagPocket_PokeBalls", 0);
   #endif
   for (i = 0; i < BAG_POKEBALLS_COUNT; ++i) {
      lu_BitstreamWrite_ItemSlot(&state, &p_SaveBlock1->bagPocket_PokeBalls[i]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->bagPocket_TMHM", 0);
   #endif
   for (i = 0; i < BAG_TMHM_COUNT; ++i) {
      lu_BitstreamWrite_ItemSlot(&state, &p_SaveBlock1->bagPocket_TMHM[i]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->bagPocket_Berries", 0);
   #endif
   for (i = 0; i < BAG_BERRIES_COUNT; ++i) {
      lu_BitstreamWrite_ItemSlot(&state, &p_SaveBlock1->bagPocket_Berries[i]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->pokeblocks", 0);
   #endif
   for (i = 0; i < POKEBLOCKS_COUNT; ++i) {
      lu_BitstreamWrite_Pokeblock(&state, &p_SaveBlock1->pokeblocks[i]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->seen1", 0);
   #endif
   for (i = 0; i < NUM_DEX_FLAG_BYTES; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->seen1[i], 8);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->berryBlenderRecords", 0);
   #endif
   for (i = 0; i < 3; ++i) {
      lu_BitstreamWrite_u16(&state, p_SaveBlock1->berryBlenderRecords[i], 16);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->trainerRematchStepCounter", 0);
   #endif
   lu_BitstreamWrite_u16(&state, p_SaveBlock1->trainerRematchStepCounter, 16);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->trainerRematches", 0);
   #endif
   for (i = 0; i < MAX_REMATCH_ENTRIES; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->trainerRematches[i], 8);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->objectEvents", 0);
   #endif
   for (i = 0; i < OBJECT_EVENTS_COUNT; ++i) {
      lu_BitstreamWrite_ObjectEvent(&state, &p_SaveBlock1->objectEvents[i]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->objectEventTemplates", 0);
   #endif
   for (i = 0; i < OBJECT_EVENT_TEMPLATES_COUNT; ++i) {
      lu_BitstreamWrite_ObjectEventTemplate(&state, &p_SaveBlock1->objectEventTemplates[i]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->flags", 0);
   #endif
   for (i = 0; i < 62; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->flags[i], 8);
   }
};

void lu_WriteSaveSector_WorldData01(u8* dst, const struct SaveBlock1* p_SaveBlock1) {
   u16 i;
   struct lu_BitstreamState state;
   lu_BitstreamInitialize(&state, dst);

   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->flags", 0);
   #endif
   for (i = 0; i < 300; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->flags[i], 8);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->vars", 0);
   #endif
   for (i = 0; i < VARS_COUNT; ++i) {
      lu_BitstreamWrite_u16(&state, p_SaveBlock1->vars[i], 16);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->gameStats", 0);
   #endif
   for (i = 0; i < NUM_GAME_STATS; ++i) {
      lu_BitstreamWrite_u32(&state, p_SaveBlock1->gameStats[i], 32);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->berryTrees", 0);
   #endif
   for (i = 0; i < BERRY_TREES_COUNT; ++i) {
      lu_BitstreamWrite_BerryTree(&state, &p_SaveBlock1->berryTrees[i]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->secretBases", 0);
   #endif
   for (i = 0; i < 14; ++i) {
      lu_BitstreamWrite_SecretBase(&state, &p_SaveBlock1->secretBases[i]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->secretBases[14].secretBaseId", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->secretBases[14].secretBaseId, 8);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->secretBases[14].toRegister", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->secretBases[14].toRegister, 4);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->secretBases[14].gender", 0);
   #endif
   lu_BitstreamWrite_bool(&state, p_SaveBlock1->secretBases[14].gender);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->secretBases[14].battledOwnerToday", 0);
   #endif
   lu_BitstreamWrite_bool(&state, p_SaveBlock1->secretBases[14].battledOwnerToday);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->secretBases[14].registryStatus", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->secretBases[14].registryStatus, 2);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->secretBases[14].trainerName", 0);
   #endif
   lu_BitstreamWrite_string_optional_terminator(&state, p_SaveBlock1->secretBases[14].trainerName, PLAYER_NAME_LENGTH);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->secretBases[14].trainerId", 0);
   #endif
   for (i = 0; i < TRAINER_ID_LENGTH; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->secretBases[14].trainerId[i], 8);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->secretBases[14].language", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->secretBases[14].language, 3);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->secretBases[14].numSecretBasesReceived", 0);
   #endif
   lu_BitstreamWrite_u16(&state, p_SaveBlock1->secretBases[14].numSecretBasesReceived, 16);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->secretBases[14].numTimesEntered", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->secretBases[14].numTimesEntered, 8);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->secretBases[14].unused", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->secretBases[14].unused, 8);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->secretBases[14].decorations", 0);
   #endif
   for (i = 0; i < DECOR_MAX_SECRET_BASE; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->secretBases[14].decorations[i], 8);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->secretBases[14].decorationPositions", 0);
   #endif
   for (i = 0; i < DECOR_MAX_SECRET_BASE; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->secretBases[14].decorationPositions[i], 8);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->secretBases[14].party.personality", 0);
   #endif
   for (i = 0; i < PARTY_SIZE; ++i) {
      lu_BitstreamWrite_u32(&state, p_SaveBlock1->secretBases[14].party.personality[i], 32);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->secretBases[14].party.moves", 0);
   #endif
   for (i = 0; i < PARTY_SIZE * MAX_MON_MOVES; ++i) {
      lu_BitstreamWrite_u16(&state, p_SaveBlock1->secretBases[14].party.moves[i], 16);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->secretBases[14].party.species", 0);
   #endif
   for (i = 0; i < 3; ++i) {
      lu_BitstreamWrite_u16(&state, p_SaveBlock1->secretBases[14].party.species[i], 11);
   }
};

void lu_WriteSaveSector_WorldData02(u8* dst, const struct SaveBlock1* p_SaveBlock1) {
   u8 i;
   struct lu_BitstreamState state;
   lu_BitstreamInitialize(&state, dst);

   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->secretBases[14].party.species", 0);
   #endif
   for (i = 0; i < 6; ++i) {
      lu_BitstreamWrite_u16(&state, p_SaveBlock1->secretBases[14].party.species[i], 11);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->secretBases[14].party.heldItems", 0);
   #endif
   for (i = 0; i < PARTY_SIZE; ++i) {
      lu_BitstreamWrite_u16(&state, p_SaveBlock1->secretBases[14].party.heldItems[i], 9);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->secretBases[14].party.levels", 0);
   #endif
   for (i = 0; i < PARTY_SIZE; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->secretBases[14].party.levels[i], 7);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->secretBases[14].party.EVs", 0);
   #endif
   for (i = 0; i < PARTY_SIZE; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->secretBases[14].party.EVs[i], 8);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->secretBases", 0);
   #endif
   for (i = 0; i < 20; ++i) {
      lu_BitstreamWrite_SecretBase(&state, &p_SaveBlock1->secretBases[i]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->playerRoomDecorations", 0);
   #endif
   for (i = 0; i < DECOR_MAX_PLAYERS_HOUSE; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->playerRoomDecorations[i], 8);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->playerRoomDecorationPositions", 0);
   #endif
   for (i = 0; i < DECOR_MAX_PLAYERS_HOUSE; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->playerRoomDecorationPositions[i], 8);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->decorationDesks", 0);
   #endif
   for (i = 0; i < 10; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->decorationDesks[i], 8);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->decorationChairs", 0);
   #endif
   for (i = 0; i < 10; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->decorationChairs[i], 8);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->decorationPlants", 0);
   #endif
   for (i = 0; i < 10; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->decorationPlants[i], 8);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->decorationOrnaments", 0);
   #endif
   for (i = 0; i < 30; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->decorationOrnaments[i], 8);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->decorationMats", 0);
   #endif
   for (i = 0; i < 30; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->decorationMats[i], 8);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->decorationPosters", 0);
   #endif
   for (i = 0; i < 10; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->decorationPosters[i], 8);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->decorationDolls", 0);
   #endif
   for (i = 0; i < 40; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->decorationDolls[i], 8);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->decorationCushions", 0);
   #endif
   for (i = 0; i < 10; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->decorationCushions[i], 8);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->tvShows", 0);
   #endif
   for (i = 0; i < TV_SHOWS_COUNT; ++i) {
      lu_BitstreamWrite_buffer(&state, &p_SaveBlock1->tvShows[i], 36);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->pokeNews", 0);
   #endif
   for (i = 0; i < POKE_NEWS_COUNT; ++i) {
      lu_BitstreamWrite_PokeNews(&state, &p_SaveBlock1->pokeNews[i]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->outbreakPokemonSpecies", 0);
   #endif
   lu_BitstreamWrite_u16(&state, p_SaveBlock1->outbreakPokemonSpecies, 11);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->outbreakLocationMapNum", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->outbreakLocationMapNum, 8);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->outbreakLocationMapGroup", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->outbreakLocationMapGroup, 8);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->outbreakPokemonLevel", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->outbreakPokemonLevel, 7);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->outbreakUnused1", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->outbreakUnused1, 8);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->outbreakUnused2", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->outbreakUnused2, 8);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->outbreakPokemonMoves", 0);
   #endif
   for (i = 0; i < MAX_MON_MOVES; ++i) {
      lu_BitstreamWrite_u16(&state, p_SaveBlock1->outbreakPokemonMoves[i], 16);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->outbreakUnused3", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->outbreakUnused3, 8);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->outbreakPokemonProbability", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->outbreakPokemonProbability, 8);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->outbreakDaysLeft", 0);
   #endif
   lu_BitstreamWrite_u16(&state, p_SaveBlock1->outbreakDaysLeft, 16);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->gabbyAndTyData", 0);
   #endif
   lu_BitstreamWrite_GabbyAndTyData(&state, &p_SaveBlock1->gabbyAndTyData);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->easyChatProfile", 0);
   #endif
   for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) {
      lu_BitstreamWrite_u16(&state, p_SaveBlock1->easyChatProfile[i], 16);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->easyChatBattleStart", 0);
   #endif
   for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) {
      lu_BitstreamWrite_u16(&state, p_SaveBlock1->easyChatBattleStart[i], 16);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->easyChatBattleWon", 0);
   #endif
   for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) {
      lu_BitstreamWrite_u16(&state, p_SaveBlock1->easyChatBattleWon[i], 16);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->easyChatBattleLost", 0);
   #endif
   for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) {
      lu_BitstreamWrite_u16(&state, p_SaveBlock1->easyChatBattleLost[i], 16);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->mail", 0);
   #endif
   for (i = 0; i < MAIL_COUNT; ++i) {
      lu_BitstreamWrite_Mail(&state, &p_SaveBlock1->mail[i]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->unlockedTrendySayings", 0);
   #endif
   for (i = 0; i < NUM_TRENDY_SAYING_BYTES; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->unlockedTrendySayings[i], 8);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->oldMan", 0);
   #endif
   lu_BitstreamWrite_buffer(&state, &p_SaveBlock1->oldMan, 64);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->dewfordTrends", 0);
   #endif
   for (i = 0; i < SAVED_TRENDS_COUNT; ++i) {
      lu_BitstreamWrite_DewfordTrend(&state, &p_SaveBlock1->dewfordTrends[i]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->contestWinners", 0);
   #endif
   for (i = 0; i < NUM_CONTEST_WINNERS; ++i) {
      lu_BitstreamWrite_ContestWinner(&state, &p_SaveBlock1->contestWinners[i]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->daycare", 0);
   #endif
   lu_BitstreamWrite_DayCare(&state, &p_SaveBlock1->daycare);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->linkBattleRecords", 0);
   #endif
   lu_BitstreamWrite_LinkBattleRecords(&state, &p_SaveBlock1->linkBattleRecords);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->giftRibbons", 0);
   #endif
   for (i = 0; i < GIFT_RIBBONS_COUNT; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->giftRibbons[i], 8);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->externalEventData", 0);
   #endif
   lu_BitstreamWrite_ExternalEventData(&state, &p_SaveBlock1->externalEventData);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->externalEventFlags", 0);
   #endif
   lu_BitstreamWrite_ExternalEventFlags(&state, &p_SaveBlock1->externalEventFlags);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->roamer", 0);
   #endif
   lu_BitstreamWrite_Roamer(&state, &p_SaveBlock1->roamer);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->enigmaBerry", 0);
   #endif
   lu_BitstreamWrite_EnigmaBerry(&state, &p_SaveBlock1->enigmaBerry);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->mysteryGift.newsCrc", 0);
   #endif
   lu_BitstreamWrite_u32(&state, p_SaveBlock1->mysteryGift.newsCrc, 32);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->mysteryGift.news", 0);
   #endif
   lu_BitstreamWrite_WonderNews(&state, &p_SaveBlock1->mysteryGift.news);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->mysteryGift.cardCrc", 0);
   #endif
   lu_BitstreamWrite_u32(&state, p_SaveBlock1->mysteryGift.cardCrc, 32);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->mysteryGift.card.flagId", 0);
   #endif
   lu_BitstreamWrite_u16(&state, p_SaveBlock1->mysteryGift.card.flagId, 16);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->mysteryGift.card.iconSpecies", 0);
   #endif
   lu_BitstreamWrite_u16(&state, p_SaveBlock1->mysteryGift.card.iconSpecies, 11);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->mysteryGift.card.idNumber", 0);
   #endif
   lu_BitstreamWrite_u32(&state, p_SaveBlock1->mysteryGift.card.idNumber, 32);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->mysteryGift.card.type", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->mysteryGift.card.type, 2);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->mysteryGift.card.bgType", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->mysteryGift.card.bgType, 4);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->mysteryGift.card.sendType", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->mysteryGift.card.sendType, 2);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->mysteryGift.card.maxStamps", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_SaveBlock1->mysteryGift.card.maxStamps, 8);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->mysteryGift.card.titleText", 0);
   #endif
   lu_BitstreamWrite_string_optional_terminator(&state, p_SaveBlock1->mysteryGift.card.titleText, WONDER_CARD_TEXT_LENGTH);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->mysteryGift.card.subtitleText", 0);
   #endif
   lu_BitstreamWrite_string_optional_terminator(&state, p_SaveBlock1->mysteryGift.card.subtitleText, WONDER_CARD_TEXT_LENGTH);
};

void lu_WriteSaveSector_WorldData03(u8* dst, const struct SaveBlock1* p_SaveBlock1) {
   u8 i, j;
   struct lu_BitstreamState state;
   lu_BitstreamInitialize(&state, dst);

   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->mysteryGift.card.bodyText", 0);
   #endif
   for (i = 0; i < 4; ++i) {
      lu_BitstreamWrite_string_optional_terminator(&state, p_SaveBlock1->mysteryGift.card.bodyText[i], WONDER_CARD_TEXT_LENGTH);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->mysteryGift.card.footerLine1Text", 0);
   #endif
   lu_BitstreamWrite_string_optional_terminator(&state, p_SaveBlock1->mysteryGift.card.footerLine1Text, WONDER_CARD_TEXT_LENGTH);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->mysteryGift.card.footerLine2Text", 0);
   #endif
   lu_BitstreamWrite_string_optional_terminator(&state, p_SaveBlock1->mysteryGift.card.footerLine2Text, WONDER_CARD_TEXT_LENGTH);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->mysteryGift.cardMetadataCrc", 0);
   #endif
   lu_BitstreamWrite_u32(&state, p_SaveBlock1->mysteryGift.cardMetadataCrc, 32);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->mysteryGift.cardMetadata", 0);
   #endif
   lu_BitstreamWrite_WonderCardMetadata(&state, &p_SaveBlock1->mysteryGift.cardMetadata);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->mysteryGift.questionnaireWords", 0);
   #endif
   for (i = 0; i < NUM_QUESTIONNAIRE_WORDS; ++i) {
      lu_BitstreamWrite_u16(&state, p_SaveBlock1->mysteryGift.questionnaireWords[i], 16);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->mysteryGift.newsMetadata", 0);
   #endif
   lu_BitstreamWrite_WonderNewsMetadata(&state, &p_SaveBlock1->mysteryGift.newsMetadata);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->mysteryGift.trainerIds", 0);
   #endif
   for (i = 0; i < 2; ++i) {
      for (j = 0; j < 5; ++j) {
         lu_BitstreamWrite_u32(&state, p_SaveBlock1->mysteryGift.trainerIds[i][j], 32);
      }
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->trainerHillTimes", 0);
   #endif
   for (i = 0; i < NUM_TRAINER_HILL_MODES; ++i) {
      lu_BitstreamWrite_u32(&state, p_SaveBlock1->trainerHillTimes[i], 32);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->ramScript", 0);
   #endif
   lu_BitstreamWrite_RamScript(&state, &p_SaveBlock1->ramScript);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->recordMixingGift", 0);
   #endif
   lu_BitstreamWrite_RecordMixingGift(&state, &p_SaveBlock1->recordMixingGift);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->seen2", 0);
   #endif
   for (i = 0; i < NUM_DEX_FLAG_BYTES; ++i) {
      lu_BitstreamWrite_u8(&state, p_SaveBlock1->seen2[i], 8);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->lilycoveLady", 0);
   #endif
   lu_BitstreamWrite_buffer(&state, &p_SaveBlock1->lilycoveLady, 64);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->trainerNameRecords", 0);
   #endif
   for (i = 0; i < 20; ++i) {
      lu_BitstreamWrite_TrainerNameRecord(&state, &p_SaveBlock1->trainerNameRecords[i]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->registeredTexts", 0);
   #endif
   for (i = 0; i < UNION_ROOM_KB_ROW_COUNT; ++i) {
      for (j = 0; j < 21; ++j) {
         lu_BitstreamWrite_u8(&state, p_SaveBlock1->registeredTexts[i][j], 8);
      }
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->trainerHill", 0);
   #endif
   lu_BitstreamWrite_TrainerHillSave(&state, &p_SaveBlock1->trainerHill);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock1->waldaPhrase", 0);
   #endif
   lu_BitstreamWrite_WaldaPhrase(&state, &p_SaveBlock1->waldaPhrase);
};

