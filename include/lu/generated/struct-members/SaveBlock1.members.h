// generated

   struct Coords16 pos;
   struct WarpData location;
   struct WarpData continueGameWarp;
   struct WarpData dynamicWarp;
   struct WarpData lastHealLocation; // used by white-out and teleport
   struct WarpData escapeWarp; // used by Dig and Escape Rope
   u16 savedMusic;
   u8 weather;
   u8 weatherCycleStage;
   u8 flashLevel;
   u16 mapLayoutId;
   u16 mapView[0x100];
   u8 playerPartyCount;
   struct Pokemon playerParty[PARTY_SIZE];
   u32 money; // Encrypted using SaveBlock2::encryptionKey; see `ApplyNewEncryptionKeyToAllEncryptedData` in `load_save.c`
   u16 coins; // Encrypted using SaveBlock2::encryptionKey; see `ApplyNewEncryptionKeyToAllEncryptedData` in `load_save.c`
   u16 registeredItem;
   struct ItemSlot pcItems[PC_ITEMS_COUNT];
   struct ItemSlot bagPocket_Items[BAG_ITEMS_COUNT];
   struct ItemSlot bagPocket_KeyItems[BAG_KEYITEMS_COUNT];
   struct ItemSlot bagPocket_PokeBalls[BAG_POKEBALLS_COUNT];
   struct ItemSlot bagPocket_TMHM[BAG_TMHM_COUNT];
   struct ItemSlot bagPocket_Berries[BAG_BERRIES_COUNT];
   struct Pokeblock pokeblocks[POKEBLOCKS_COUNT];
   u8 seen1[NUM_DEX_FLAG_BYTES];
   u16 berryBlenderRecords[3];
   u8 unused_9C2[6];
   u16 trainerRematchStepCounter;
   u8 trainerRematches[MAX_REMATCH_ENTRIES];
   struct ObjectEvent objectEvents[OBJECT_EVENTS_COUNT];
   struct ObjectEventTemplate objectEventTemplates[OBJECT_EVENT_TEMPLATES_COUNT];
   u8 flags[NUM_FLAG_BYTES];
   u16 vars[VARS_COUNT];
   u32 gameStats[NUM_GAME_STATS]; // Encrypted using SaveBlock2::encryptionKey; see `ApplyNewEncryptionKeyToGameStats` in `overworld.c`
   struct BerryTree berryTrees[BERRY_TREES_COUNT];
   struct SecretBase secretBases[SECRET_BASES_COUNT];
   u8 playerRoomDecorations[DECOR_MAX_PLAYERS_HOUSE];
   u8 playerRoomDecorationPositions[DECOR_MAX_PLAYERS_HOUSE];
   u8 decorationDesks[10];
   u8 decorationChairs[10];
   u8 decorationPlants[10];
   u8 decorationOrnaments[30];
   u8 decorationMats[30];
   u8 decorationPosters[10];
   u8 decorationDolls[40];
   u8 decorationCushions[10];
   TVShow tvShows[TV_SHOWS_COUNT];
   PokeNews pokeNews[POKE_NEWS_COUNT];
   u16 outbreakPokemonSpecies;
   u8 outbreakLocationMapNum;
   u8 outbreakLocationMapGroup;
   u8 outbreakPokemonLevel;
   u8 outbreakUnused1;
   u8 outbreakUnused2;
   u16 outbreakPokemonMoves[MAX_MON_MOVES];
   u8 outbreakUnused3;
   u8 outbreakPokemonProbability;
   u16 outbreakDaysLeft;
   struct GabbyAndTyData gabbyAndTyData;
   u16 easyChatProfile[EASY_CHAT_BATTLE_WORDS_COUNT];
   u16 easyChatBattleStart[EASY_CHAT_BATTLE_WORDS_COUNT];
   u16 easyChatBattleWon[EASY_CHAT_BATTLE_WORDS_COUNT];
   u16 easyChatBattleLost[EASY_CHAT_BATTLE_WORDS_COUNT];
   struct Mail mail[MAIL_COUNT];
   u8 unlockedTrendySayings[NUM_TRENDY_SAYING_BYTES]; // Bitfield for unlockable Easy Chat words in EC_GROUP_TRENDY_SAYING
   OldMan oldMan;
   struct DewfordTrend dewfordTrends[SAVED_TRENDS_COUNT];
   struct ContestWinner contestWinners[NUM_CONTEST_WINNERS];
   struct DayCare daycare;
   struct LinkBattleRecords linkBattleRecords;
   u8 giftRibbons[GIFT_RIBBONS_COUNT];
   struct ExternalEventData externalEventData;
   struct ExternalEventFlags externalEventFlags;
   struct Roamer roamer;
   struct EnigmaBerry enigmaBerry;
   struct MysteryGiftSave mysteryGift;
   u8 unused_3598[0x180];
   u32 trainerHillTimes[NUM_TRAINER_HILL_MODES];
   struct RamScript ramScript;
   struct RecordMixingGift recordMixingGift;
   u8 seen2[NUM_DEX_FLAG_BYTES];
   LilycoveLady lilycoveLady;
   struct TrainerNameRecord trainerNameRecords[20];
   u8 registeredTexts[UNION_ROOM_KB_ROW_COUNT][21];
   u8 unused_3D5A[10];
   struct TrainerHillSave trainerHill;
   struct WaldaPhrase waldaPhrase;