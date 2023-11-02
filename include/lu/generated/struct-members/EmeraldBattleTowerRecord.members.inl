   u8 lvlMode; // 0 = level 50, 1 = level 100
   u8 facilityClass;
   u16 winStreak;
   u8 name[PLAYER_NAME_LENGTH + 1];
   u8 trainerId[TRAINER_ID_LENGTH];
   u16 greeting[EASY_CHAT_BATTLE_WORDS_COUNT];
   u16 speechWon[EASY_CHAT_BATTLE_WORDS_COUNT];
   u16 speechLost[EASY_CHAT_BATTLE_WORDS_COUNT];
   struct BattleTowerPokemon party[MAX_FRONTIER_PARTY_SIZE];
   u8 language;
   u32 checksum;
