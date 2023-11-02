   u8 playerName[PLAYER_NAME_LENGTH + 1];
   u8 playerGender;
   u8 specialSaveWarpFlags;
   u8 playerTrainerId[TRAINER_ID_LENGTH];
   u16 playTimeHours;
   u8 playTimeMinutes;
   u8 playTimeSeconds;
   u8 playTimeVBlanks;
   u8 optionsButtonMode; // OPTIONS_BUTTON_MODE_[NORMAL/LR/L_EQUALS_A]
   u16 optionsTextSpeed : 3; // OPTIONS_TEXT_SPEED_[SLOW/MID/FAST]
   u16 optionsWindowFrameType : 5; // Specifies one of the 20 decorative borders for text boxes
   u16 optionsSound : 1; // OPTIONS_SOUND_[MONO/STERE
   u16 optionsBattleStyle : 1; // OPTIONS_BATTLE_STYLE_[SHIFT/SET]
   u16 optionsBattleSceneOff : 1; // whether battle animations are disabled
   u16 regionMapZoom : 1; // whether the map is zoomed in
   u16 optionsRunningToggle : 1; // Lu: Running Shoes option
   struct Pokedex pokedex;
   u8 filler_90[8];
   struct Time localTimeOffset;
   struct Time lastBerryTreeUpdate;
   u32 gcnLinkFlags; // Read by Pokemon Colosseum/XD
   u32 encryptionKey;
   struct PlayersApprentice playerApprentice;
   struct Apprentice apprentices[APPRENTICE_COUNT];
   struct BerryCrush berryCrush;
   struct PokemonJumpRecords pokeJump;
   struct BerryPickingResults berryPick;
   struct RankingHall1P hallRecords1P[HALL_FACILITIES_COUNT][FRONTIER_LVL_MODE_COUNT][HALL_RECORDS_COUNT]; // From record mixing.
   struct RankingHall2P hallRecords2P[FRONTIER_LVL_MODE_COUNT][HALL_RECORDS_COUNT]; // From record mixing.
   u16 contestLinkResults[CONTEST_CATEGORIES_COUNT][CONTESTANT_COUNT];
   struct BattleFrontier frontier;
