#include "lu/generated/sector-serialize/PokemonStorage.h"

#include "global.h"
#include "pokemon_storage_system.h"
// whole-struct serialize funcs:
#include "lu/generated/struct-serialize/serialize_BoxPokemon.h"

void lu_ReadSaveSector_PokemonStorage00(const u8* src, struct PokemonStorage* p_PokemonStorage) {
   u8 i, j;
   struct lu_BitstreamState state;
   lu_BitstreamInitialize(&state, (u8*)src); // need to cast away constness to store it here

   p_PokemonStorage->currentBox = lu_BitstreamRead_u8(&state, 4);
   for (j = 0; j < IN_BOX_COUNT; ++j) {
      lu_BitstreamRead_BoxPokemon(&state, &p_PokemonStorage->boxes[0][j]);
   }
   for (j = 0; j < 19; ++j) {
      lu_BitstreamRead_BoxPokemon(&state, &p_PokemonStorage->boxes[1][j]);
   }
   p_PokemonStorage->boxes[1][19].personality = lu_BitstreamRead_u32(&state, 32);
   p_PokemonStorage->boxes[1][19].otId = lu_BitstreamRead_u32(&state, 32);
   lu_BitstreamRead_string_optional_terminator(&state, p_PokemonStorage->boxes[1][19].nickname, POKEMON_NAME_LENGTH);
   p_PokemonStorage->boxes[1][19].language = lu_BitstreamRead_u8(&state, 3);
   p_PokemonStorage->boxes[1][19].isBadEgg = lu_BitstreamRead_bool(&state);
   p_PokemonStorage->boxes[1][19].hasSpecies = lu_BitstreamRead_bool(&state);
   p_PokemonStorage->boxes[1][19].isEgg = lu_BitstreamRead_bool(&state);
   p_PokemonStorage->boxes[1][19].unused = lu_BitstreamRead_u8(&state, 5);
   lu_BitstreamRead_string_optional_terminator(&state, p_PokemonStorage->boxes[1][19].otName, PLAYER_NAME_LENGTH);
   p_PokemonStorage->boxes[1][19].markings = lu_BitstreamRead_u8(&state, 8);
   p_PokemonStorage->boxes[1][19].checksum = lu_BitstreamRead_u16(&state, 16);
   p_PokemonStorage->boxes[1][19].unknown = lu_BitstreamRead_u16(&state, 16);
   for (i = 0; i < 11; ++i) {
      p_PokemonStorage->boxes[1][19].secure.raw[i] = lu_BitstreamRead_u32(&state, 32);
   }
};

void lu_ReadSaveSector_PokemonStorage01(const u8* src, struct PokemonStorage* p_PokemonStorage) {
   u8 i, j;
   struct lu_BitstreamState state;
   lu_BitstreamInitialize(&state, (u8*)src); // need to cast away constness to store it here

   p_PokemonStorage->boxes[1][19].secure.raw[11] = lu_BitstreamRead_u32(&state, 32);
   for (j = 20; j < 30; ++j) {
      lu_BitstreamRead_BoxPokemon(&state, &p_PokemonStorage->boxes[1][j]);
   }
   for (j = 0; j < IN_BOX_COUNT; ++j) {
      lu_BitstreamRead_BoxPokemon(&state, &p_PokemonStorage->boxes[2][j]);
   }
   for (j = 0; j < 9; ++j) {
      lu_BitstreamRead_BoxPokemon(&state, &p_PokemonStorage->boxes[3][j]);
   }
   p_PokemonStorage->boxes[3][9].personality = lu_BitstreamRead_u32(&state, 32);
   p_PokemonStorage->boxes[3][9].otId = lu_BitstreamRead_u32(&state, 32);
   lu_BitstreamRead_string_optional_terminator(&state, p_PokemonStorage->boxes[3][9].nickname, POKEMON_NAME_LENGTH);
   p_PokemonStorage->boxes[3][9].language = lu_BitstreamRead_u8(&state, 3);
   p_PokemonStorage->boxes[3][9].isBadEgg = lu_BitstreamRead_bool(&state);
   p_PokemonStorage->boxes[3][9].hasSpecies = lu_BitstreamRead_bool(&state);
   p_PokemonStorage->boxes[3][9].isEgg = lu_BitstreamRead_bool(&state);
   p_PokemonStorage->boxes[3][9].unused = lu_BitstreamRead_u8(&state, 5);
   lu_BitstreamRead_string_optional_terminator(&state, p_PokemonStorage->boxes[3][9].otName, PLAYER_NAME_LENGTH);
   p_PokemonStorage->boxes[3][9].markings = lu_BitstreamRead_u8(&state, 8);
   p_PokemonStorage->boxes[3][9].checksum = lu_BitstreamRead_u16(&state, 16);
   p_PokemonStorage->boxes[3][9].unknown = lu_BitstreamRead_u16(&state, 16);
   for (i = 0; i < 10; ++i) {
      p_PokemonStorage->boxes[3][9].secure.raw[i] = lu_BitstreamRead_u32(&state, 32);
   }
};

void lu_ReadSaveSector_PokemonStorage02(const u8* src, struct PokemonStorage* p_PokemonStorage) {
   u8 i, j;
   struct lu_BitstreamState state;
   lu_BitstreamInitialize(&state, (u8*)src); // need to cast away constness to store it here

   for (i = 10; i < 12; ++i) {
      p_PokemonStorage->boxes[3][9].secure.raw[i] = lu_BitstreamRead_u32(&state, 32);
   }
   for (j = 10; j < 30; ++j) {
      lu_BitstreamRead_BoxPokemon(&state, &p_PokemonStorage->boxes[3][j]);
   }
   for (j = 0; j < 29; ++j) {
      lu_BitstreamRead_BoxPokemon(&state, &p_PokemonStorage->boxes[4][j]);
   }
   p_PokemonStorage->boxes[4][29].personality = lu_BitstreamRead_u32(&state, 32);
   p_PokemonStorage->boxes[4][29].otId = lu_BitstreamRead_u32(&state, 32);
   lu_BitstreamRead_string_optional_terminator(&state, p_PokemonStorage->boxes[4][29].nickname, POKEMON_NAME_LENGTH);
   p_PokemonStorage->boxes[4][29].language = lu_BitstreamRead_u8(&state, 3);
   p_PokemonStorage->boxes[4][29].isBadEgg = lu_BitstreamRead_bool(&state);
   p_PokemonStorage->boxes[4][29].hasSpecies = lu_BitstreamRead_bool(&state);
   p_PokemonStorage->boxes[4][29].isEgg = lu_BitstreamRead_bool(&state);
   p_PokemonStorage->boxes[4][29].unused = lu_BitstreamRead_u8(&state, 5);
   lu_BitstreamRead_string_optional_terminator(&state, p_PokemonStorage->boxes[4][29].otName, PLAYER_NAME_LENGTH);
   p_PokemonStorage->boxes[4][29].markings = lu_BitstreamRead_u8(&state, 8);
   p_PokemonStorage->boxes[4][29].checksum = lu_BitstreamRead_u16(&state, 16);
   p_PokemonStorage->boxes[4][29].unknown = lu_BitstreamRead_u16(&state, 16);
   for (i = 0; i < 9; ++i) {
      p_PokemonStorage->boxes[4][29].secure.raw[i] = lu_BitstreamRead_u32(&state, 32);
   }
};

void lu_ReadSaveSector_PokemonStorage03(const u8* src, struct PokemonStorage* p_PokemonStorage) {
   u8 i, j;
   struct lu_BitstreamState state;
   lu_BitstreamInitialize(&state, (u8*)src); // need to cast away constness to store it here

   for (i = 9; i < 12; ++i) {
      p_PokemonStorage->boxes[4][29].secure.raw[i] = lu_BitstreamRead_u32(&state, 32);
   }
   for (j = 0; j < IN_BOX_COUNT; ++j) {
      lu_BitstreamRead_BoxPokemon(&state, &p_PokemonStorage->boxes[5][j]);
   }
   for (j = 0; j < 19; ++j) {
      lu_BitstreamRead_BoxPokemon(&state, &p_PokemonStorage->boxes[6][j]);
   }
   p_PokemonStorage->boxes[6][19].personality = lu_BitstreamRead_u32(&state, 32);
   p_PokemonStorage->boxes[6][19].otId = lu_BitstreamRead_u32(&state, 32);
   lu_BitstreamRead_string_optional_terminator(&state, p_PokemonStorage->boxes[6][19].nickname, POKEMON_NAME_LENGTH);
   p_PokemonStorage->boxes[6][19].language = lu_BitstreamRead_u8(&state, 3);
   p_PokemonStorage->boxes[6][19].isBadEgg = lu_BitstreamRead_bool(&state);
   p_PokemonStorage->boxes[6][19].hasSpecies = lu_BitstreamRead_bool(&state);
   p_PokemonStorage->boxes[6][19].isEgg = lu_BitstreamRead_bool(&state);
   p_PokemonStorage->boxes[6][19].unused = lu_BitstreamRead_u8(&state, 5);
   lu_BitstreamRead_string_optional_terminator(&state, p_PokemonStorage->boxes[6][19].otName, PLAYER_NAME_LENGTH);
   p_PokemonStorage->boxes[6][19].markings = lu_BitstreamRead_u8(&state, 8);
   p_PokemonStorage->boxes[6][19].checksum = lu_BitstreamRead_u16(&state, 16);
   p_PokemonStorage->boxes[6][19].unknown = lu_BitstreamRead_u16(&state, 16);
   for (i = 0; i < 8; ++i) {
      p_PokemonStorage->boxes[6][19].secure.raw[i] = lu_BitstreamRead_u32(&state, 32);
   }
};

void lu_ReadSaveSector_PokemonStorage04(const u8* src, struct PokemonStorage* p_PokemonStorage) {
   u8 i, j;
   struct lu_BitstreamState state;
   lu_BitstreamInitialize(&state, (u8*)src); // need to cast away constness to store it here

   for (i = 8; i < 12; ++i) {
      p_PokemonStorage->boxes[6][19].secure.raw[i] = lu_BitstreamRead_u32(&state, 32);
   }
   for (j = 20; j < 30; ++j) {
      lu_BitstreamRead_BoxPokemon(&state, &p_PokemonStorage->boxes[6][j]);
   }
   for (j = 0; j < IN_BOX_COUNT; ++j) {
      lu_BitstreamRead_BoxPokemon(&state, &p_PokemonStorage->boxes[7][j]);
   }
   for (j = 0; j < 9; ++j) {
      lu_BitstreamRead_BoxPokemon(&state, &p_PokemonStorage->boxes[8][j]);
   }
   p_PokemonStorage->boxes[8][9].personality = lu_BitstreamRead_u32(&state, 32);
   p_PokemonStorage->boxes[8][9].otId = lu_BitstreamRead_u32(&state, 32);
   lu_BitstreamRead_string_optional_terminator(&state, p_PokemonStorage->boxes[8][9].nickname, POKEMON_NAME_LENGTH);
   p_PokemonStorage->boxes[8][9].language = lu_BitstreamRead_u8(&state, 3);
   p_PokemonStorage->boxes[8][9].isBadEgg = lu_BitstreamRead_bool(&state);
   p_PokemonStorage->boxes[8][9].hasSpecies = lu_BitstreamRead_bool(&state);
   p_PokemonStorage->boxes[8][9].isEgg = lu_BitstreamRead_bool(&state);
   p_PokemonStorage->boxes[8][9].unused = lu_BitstreamRead_u8(&state, 5);
   lu_BitstreamRead_string_optional_terminator(&state, p_PokemonStorage->boxes[8][9].otName, PLAYER_NAME_LENGTH);
   p_PokemonStorage->boxes[8][9].markings = lu_BitstreamRead_u8(&state, 8);
   p_PokemonStorage->boxes[8][9].checksum = lu_BitstreamRead_u16(&state, 16);
   p_PokemonStorage->boxes[8][9].unknown = lu_BitstreamRead_u16(&state, 16);
   for (i = 0; i < 7; ++i) {
      p_PokemonStorage->boxes[8][9].secure.raw[i] = lu_BitstreamRead_u32(&state, 32);
   }
};

void lu_ReadSaveSector_PokemonStorage05(const u8* src, struct PokemonStorage* p_PokemonStorage) {
   u8 i, j;
   struct lu_BitstreamState state;
   lu_BitstreamInitialize(&state, (u8*)src); // need to cast away constness to store it here

   for (i = 7; i < 12; ++i) {
      p_PokemonStorage->boxes[8][9].secure.raw[i] = lu_BitstreamRead_u32(&state, 32);
   }
   for (j = 10; j < 30; ++j) {
      lu_BitstreamRead_BoxPokemon(&state, &p_PokemonStorage->boxes[8][j]);
   }
   for (j = 0; j < 29; ++j) {
      lu_BitstreamRead_BoxPokemon(&state, &p_PokemonStorage->boxes[9][j]);
   }
   p_PokemonStorage->boxes[9][29].personality = lu_BitstreamRead_u32(&state, 32);
   p_PokemonStorage->boxes[9][29].otId = lu_BitstreamRead_u32(&state, 32);
   lu_BitstreamRead_string_optional_terminator(&state, p_PokemonStorage->boxes[9][29].nickname, POKEMON_NAME_LENGTH);
   p_PokemonStorage->boxes[9][29].language = lu_BitstreamRead_u8(&state, 3);
   p_PokemonStorage->boxes[9][29].isBadEgg = lu_BitstreamRead_bool(&state);
   p_PokemonStorage->boxes[9][29].hasSpecies = lu_BitstreamRead_bool(&state);
   p_PokemonStorage->boxes[9][29].isEgg = lu_BitstreamRead_bool(&state);
   p_PokemonStorage->boxes[9][29].unused = lu_BitstreamRead_u8(&state, 5);
   lu_BitstreamRead_string_optional_terminator(&state, p_PokemonStorage->boxes[9][29].otName, PLAYER_NAME_LENGTH);
   p_PokemonStorage->boxes[9][29].markings = lu_BitstreamRead_u8(&state, 8);
   p_PokemonStorage->boxes[9][29].checksum = lu_BitstreamRead_u16(&state, 16);
   p_PokemonStorage->boxes[9][29].unknown = lu_BitstreamRead_u16(&state, 16);
   for (i = 0; i < 6; ++i) {
      p_PokemonStorage->boxes[9][29].secure.raw[i] = lu_BitstreamRead_u32(&state, 32);
   }
};

void lu_ReadSaveSector_PokemonStorage06(const u8* src, struct PokemonStorage* p_PokemonStorage) {
   u8 i, j;
   struct lu_BitstreamState state;
   lu_BitstreamInitialize(&state, (u8*)src); // need to cast away constness to store it here

   for (i = 6; i < 12; ++i) {
      p_PokemonStorage->boxes[9][29].secure.raw[i] = lu_BitstreamRead_u32(&state, 32);
   }
   for (j = 0; j < IN_BOX_COUNT; ++j) {
      lu_BitstreamRead_BoxPokemon(&state, &p_PokemonStorage->boxes[10][j]);
   }
   for (j = 0; j < 19; ++j) {
      lu_BitstreamRead_BoxPokemon(&state, &p_PokemonStorage->boxes[11][j]);
   }
   p_PokemonStorage->boxes[11][19].personality = lu_BitstreamRead_u32(&state, 32);
   p_PokemonStorage->boxes[11][19].otId = lu_BitstreamRead_u32(&state, 32);
   lu_BitstreamRead_string_optional_terminator(&state, p_PokemonStorage->boxes[11][19].nickname, POKEMON_NAME_LENGTH);
   p_PokemonStorage->boxes[11][19].language = lu_BitstreamRead_u8(&state, 3);
   p_PokemonStorage->boxes[11][19].isBadEgg = lu_BitstreamRead_bool(&state);
   p_PokemonStorage->boxes[11][19].hasSpecies = lu_BitstreamRead_bool(&state);
   p_PokemonStorage->boxes[11][19].isEgg = lu_BitstreamRead_bool(&state);
   p_PokemonStorage->boxes[11][19].unused = lu_BitstreamRead_u8(&state, 5);
   lu_BitstreamRead_string_optional_terminator(&state, p_PokemonStorage->boxes[11][19].otName, PLAYER_NAME_LENGTH);
   p_PokemonStorage->boxes[11][19].markings = lu_BitstreamRead_u8(&state, 8);
   p_PokemonStorage->boxes[11][19].checksum = lu_BitstreamRead_u16(&state, 16);
   p_PokemonStorage->boxes[11][19].unknown = lu_BitstreamRead_u16(&state, 16);
   for (i = 0; i < 5; ++i) {
      p_PokemonStorage->boxes[11][19].secure.raw[i] = lu_BitstreamRead_u32(&state, 32);
   }
};

void lu_ReadSaveSector_PokemonStorage07(const u8* src, struct PokemonStorage* p_PokemonStorage) {
   u8 i, j;
   struct lu_BitstreamState state;
   lu_BitstreamInitialize(&state, (u8*)src); // need to cast away constness to store it here

   for (i = 5; i < 12; ++i) {
      p_PokemonStorage->boxes[11][19].secure.raw[i] = lu_BitstreamRead_u32(&state, 32);
   }
   for (j = 20; j < 30; ++j) {
      lu_BitstreamRead_BoxPokemon(&state, &p_PokemonStorage->boxes[11][j]);
   }
   for (j = 0; j < IN_BOX_COUNT; ++j) {
      lu_BitstreamRead_BoxPokemon(&state, &p_PokemonStorage->boxes[12][j]);
   }
   for (j = 0; j < 9; ++j) {
      lu_BitstreamRead_BoxPokemon(&state, &p_PokemonStorage->boxes[13][j]);
   }
   p_PokemonStorage->boxes[13][9].personality = lu_BitstreamRead_u32(&state, 32);
   p_PokemonStorage->boxes[13][9].otId = lu_BitstreamRead_u32(&state, 32);
   lu_BitstreamRead_string_optional_terminator(&state, p_PokemonStorage->boxes[13][9].nickname, POKEMON_NAME_LENGTH);
   p_PokemonStorage->boxes[13][9].language = lu_BitstreamRead_u8(&state, 3);
   p_PokemonStorage->boxes[13][9].isBadEgg = lu_BitstreamRead_bool(&state);
   p_PokemonStorage->boxes[13][9].hasSpecies = lu_BitstreamRead_bool(&state);
   p_PokemonStorage->boxes[13][9].isEgg = lu_BitstreamRead_bool(&state);
   p_PokemonStorage->boxes[13][9].unused = lu_BitstreamRead_u8(&state, 5);
   lu_BitstreamRead_string_optional_terminator(&state, p_PokemonStorage->boxes[13][9].otName, PLAYER_NAME_LENGTH);
   p_PokemonStorage->boxes[13][9].markings = lu_BitstreamRead_u8(&state, 8);
   p_PokemonStorage->boxes[13][9].checksum = lu_BitstreamRead_u16(&state, 16);
   p_PokemonStorage->boxes[13][9].unknown = lu_BitstreamRead_u16(&state, 16);
   for (i = 0; i < 4; ++i) {
      p_PokemonStorage->boxes[13][9].secure.raw[i] = lu_BitstreamRead_u32(&state, 32);
   }
};

void lu_ReadSaveSector_PokemonStorage08(const u8* src, struct PokemonStorage* p_PokemonStorage) {
   u8 i, j;
   struct lu_BitstreamState state;
   lu_BitstreamInitialize(&state, (u8*)src); // need to cast away constness to store it here

   for (i = 4; i < 12; ++i) {
      p_PokemonStorage->boxes[13][9].secure.raw[i] = lu_BitstreamRead_u32(&state, 32);
   }
   for (j = 10; j < 30; ++j) {
      lu_BitstreamRead_BoxPokemon(&state, &p_PokemonStorage->boxes[13][j]);
   }
   for (i = 0; i < TOTAL_BOXES_COUNT; ++i) {
      lu_BitstreamRead_string(&state, p_PokemonStorage->boxNames[i], BOX_NAME_LENGTH);
   }
   for (i = 0; i < TOTAL_BOXES_COUNT; ++i) {
      p_PokemonStorage->boxWallpapers[i] = lu_BitstreamRead_u8(&state, 8);
   }
};

void lu_WriteSaveSector_PokemonStorage00(u8* dst, const struct PokemonStorage* p_PokemonStorage) {
   u8 i, j;
   struct lu_BitstreamState state;
   lu_BitstreamInitialize(&state, dst);

   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->currentBox", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_PokemonStorage->currentBox, 4);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes", 0);
   #endif
   for (j = 0; j < IN_BOX_COUNT; ++j) {
      lu_BitstreamWrite_BoxPokemon(&state, &p_PokemonStorage->boxes[0][j]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes", 0);
   #endif
   for (j = 0; j < 19; ++j) {
      lu_BitstreamWrite_BoxPokemon(&state, &p_PokemonStorage->boxes[1][j]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[1][19].personality", 0);
   #endif
   lu_BitstreamWrite_u32(&state, p_PokemonStorage->boxes[1][19].personality, 32);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[1][19].otId", 0);
   #endif
   lu_BitstreamWrite_u32(&state, p_PokemonStorage->boxes[1][19].otId, 32);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[1][19].nickname", 0);
   #endif
   lu_BitstreamWrite_string_optional_terminator(&state, p_PokemonStorage->boxes[1][19].nickname, POKEMON_NAME_LENGTH);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[1][19].language", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_PokemonStorage->boxes[1][19].language, 3);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[1][19].isBadEgg", 0);
   #endif
   lu_BitstreamWrite_bool(&state, p_PokemonStorage->boxes[1][19].isBadEgg);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[1][19].hasSpecies", 0);
   #endif
   lu_BitstreamWrite_bool(&state, p_PokemonStorage->boxes[1][19].hasSpecies);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[1][19].isEgg", 0);
   #endif
   lu_BitstreamWrite_bool(&state, p_PokemonStorage->boxes[1][19].isEgg);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[1][19].unused", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_PokemonStorage->boxes[1][19].unused, 5);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[1][19].otName", 0);
   #endif
   lu_BitstreamWrite_string_optional_terminator(&state, p_PokemonStorage->boxes[1][19].otName, PLAYER_NAME_LENGTH);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[1][19].markings", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_PokemonStorage->boxes[1][19].markings, 8);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[1][19].checksum", 0);
   #endif
   lu_BitstreamWrite_u16(&state, p_PokemonStorage->boxes[1][19].checksum, 16);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[1][19].unknown", 0);
   #endif
   lu_BitstreamWrite_u16(&state, p_PokemonStorage->boxes[1][19].unknown, 16);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[1][19].secure.raw", 0);
   #endif
   for (i = 0; i < 11; ++i) {
      lu_BitstreamWrite_u32(&state, p_PokemonStorage->boxes[1][19].secure.raw[i], 32);
   }
};

void lu_WriteSaveSector_PokemonStorage01(u8* dst, const struct PokemonStorage* p_PokemonStorage) {
   u8 i, j;
   struct lu_BitstreamState state;
   lu_BitstreamInitialize(&state, dst);

   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[1][19].secure.raw", 0);
   #endif
   lu_BitstreamWrite_u32(&state, p_PokemonStorage->boxes[1][19].secure.raw[11], 32);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes", 0);
   #endif
   for (j = 20; j < 30; ++j) {
      lu_BitstreamWrite_BoxPokemon(&state, &p_PokemonStorage->boxes[1][j]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes", 0);
   #endif
   for (j = 0; j < IN_BOX_COUNT; ++j) {
      lu_BitstreamWrite_BoxPokemon(&state, &p_PokemonStorage->boxes[2][j]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes", 0);
   #endif
   for (j = 0; j < 9; ++j) {
      lu_BitstreamWrite_BoxPokemon(&state, &p_PokemonStorage->boxes[3][j]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[3][9].personality", 0);
   #endif
   lu_BitstreamWrite_u32(&state, p_PokemonStorage->boxes[3][9].personality, 32);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[3][9].otId", 0);
   #endif
   lu_BitstreamWrite_u32(&state, p_PokemonStorage->boxes[3][9].otId, 32);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[3][9].nickname", 0);
   #endif
   lu_BitstreamWrite_string_optional_terminator(&state, p_PokemonStorage->boxes[3][9].nickname, POKEMON_NAME_LENGTH);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[3][9].language", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_PokemonStorage->boxes[3][9].language, 3);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[3][9].isBadEgg", 0);
   #endif
   lu_BitstreamWrite_bool(&state, p_PokemonStorage->boxes[3][9].isBadEgg);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[3][9].hasSpecies", 0);
   #endif
   lu_BitstreamWrite_bool(&state, p_PokemonStorage->boxes[3][9].hasSpecies);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[3][9].isEgg", 0);
   #endif
   lu_BitstreamWrite_bool(&state, p_PokemonStorage->boxes[3][9].isEgg);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[3][9].unused", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_PokemonStorage->boxes[3][9].unused, 5);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[3][9].otName", 0);
   #endif
   lu_BitstreamWrite_string_optional_terminator(&state, p_PokemonStorage->boxes[3][9].otName, PLAYER_NAME_LENGTH);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[3][9].markings", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_PokemonStorage->boxes[3][9].markings, 8);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[3][9].checksum", 0);
   #endif
   lu_BitstreamWrite_u16(&state, p_PokemonStorage->boxes[3][9].checksum, 16);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[3][9].unknown", 0);
   #endif
   lu_BitstreamWrite_u16(&state, p_PokemonStorage->boxes[3][9].unknown, 16);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[3][9].secure.raw", 0);
   #endif
   for (i = 0; i < 10; ++i) {
      lu_BitstreamWrite_u32(&state, p_PokemonStorage->boxes[3][9].secure.raw[i], 32);
   }
};

void lu_WriteSaveSector_PokemonStorage02(u8* dst, const struct PokemonStorage* p_PokemonStorage) {
   u8 i, j;
   struct lu_BitstreamState state;
   lu_BitstreamInitialize(&state, dst);

   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[3][9].secure.raw", 0);
   #endif
   for (i = 10; i < 12; ++i) {
      lu_BitstreamWrite_u32(&state, p_PokemonStorage->boxes[3][9].secure.raw[i], 32);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes", 0);
   #endif
   for (j = 10; j < 30; ++j) {
      lu_BitstreamWrite_BoxPokemon(&state, &p_PokemonStorage->boxes[3][j]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes", 0);
   #endif
   for (j = 0; j < 29; ++j) {
      lu_BitstreamWrite_BoxPokemon(&state, &p_PokemonStorage->boxes[4][j]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[4][29].personality", 0);
   #endif
   lu_BitstreamWrite_u32(&state, p_PokemonStorage->boxes[4][29].personality, 32);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[4][29].otId", 0);
   #endif
   lu_BitstreamWrite_u32(&state, p_PokemonStorage->boxes[4][29].otId, 32);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[4][29].nickname", 0);
   #endif
   lu_BitstreamWrite_string_optional_terminator(&state, p_PokemonStorage->boxes[4][29].nickname, POKEMON_NAME_LENGTH);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[4][29].language", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_PokemonStorage->boxes[4][29].language, 3);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[4][29].isBadEgg", 0);
   #endif
   lu_BitstreamWrite_bool(&state, p_PokemonStorage->boxes[4][29].isBadEgg);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[4][29].hasSpecies", 0);
   #endif
   lu_BitstreamWrite_bool(&state, p_PokemonStorage->boxes[4][29].hasSpecies);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[4][29].isEgg", 0);
   #endif
   lu_BitstreamWrite_bool(&state, p_PokemonStorage->boxes[4][29].isEgg);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[4][29].unused", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_PokemonStorage->boxes[4][29].unused, 5);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[4][29].otName", 0);
   #endif
   lu_BitstreamWrite_string_optional_terminator(&state, p_PokemonStorage->boxes[4][29].otName, PLAYER_NAME_LENGTH);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[4][29].markings", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_PokemonStorage->boxes[4][29].markings, 8);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[4][29].checksum", 0);
   #endif
   lu_BitstreamWrite_u16(&state, p_PokemonStorage->boxes[4][29].checksum, 16);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[4][29].unknown", 0);
   #endif
   lu_BitstreamWrite_u16(&state, p_PokemonStorage->boxes[4][29].unknown, 16);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[4][29].secure.raw", 0);
   #endif
   for (i = 0; i < 9; ++i) {
      lu_BitstreamWrite_u32(&state, p_PokemonStorage->boxes[4][29].secure.raw[i], 32);
   }
};

void lu_WriteSaveSector_PokemonStorage03(u8* dst, const struct PokemonStorage* p_PokemonStorage) {
   u8 i, j;
   struct lu_BitstreamState state;
   lu_BitstreamInitialize(&state, dst);

   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[4][29].secure.raw", 0);
   #endif
   for (i = 9; i < 12; ++i) {
      lu_BitstreamWrite_u32(&state, p_PokemonStorage->boxes[4][29].secure.raw[i], 32);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes", 0);
   #endif
   for (j = 0; j < IN_BOX_COUNT; ++j) {
      lu_BitstreamWrite_BoxPokemon(&state, &p_PokemonStorage->boxes[5][j]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes", 0);
   #endif
   for (j = 0; j < 19; ++j) {
      lu_BitstreamWrite_BoxPokemon(&state, &p_PokemonStorage->boxes[6][j]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[6][19].personality", 0);
   #endif
   lu_BitstreamWrite_u32(&state, p_PokemonStorage->boxes[6][19].personality, 32);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[6][19].otId", 0);
   #endif
   lu_BitstreamWrite_u32(&state, p_PokemonStorage->boxes[6][19].otId, 32);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[6][19].nickname", 0);
   #endif
   lu_BitstreamWrite_string_optional_terminator(&state, p_PokemonStorage->boxes[6][19].nickname, POKEMON_NAME_LENGTH);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[6][19].language", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_PokemonStorage->boxes[6][19].language, 3);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[6][19].isBadEgg", 0);
   #endif
   lu_BitstreamWrite_bool(&state, p_PokemonStorage->boxes[6][19].isBadEgg);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[6][19].hasSpecies", 0);
   #endif
   lu_BitstreamWrite_bool(&state, p_PokemonStorage->boxes[6][19].hasSpecies);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[6][19].isEgg", 0);
   #endif
   lu_BitstreamWrite_bool(&state, p_PokemonStorage->boxes[6][19].isEgg);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[6][19].unused", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_PokemonStorage->boxes[6][19].unused, 5);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[6][19].otName", 0);
   #endif
   lu_BitstreamWrite_string_optional_terminator(&state, p_PokemonStorage->boxes[6][19].otName, PLAYER_NAME_LENGTH);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[6][19].markings", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_PokemonStorage->boxes[6][19].markings, 8);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[6][19].checksum", 0);
   #endif
   lu_BitstreamWrite_u16(&state, p_PokemonStorage->boxes[6][19].checksum, 16);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[6][19].unknown", 0);
   #endif
   lu_BitstreamWrite_u16(&state, p_PokemonStorage->boxes[6][19].unknown, 16);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[6][19].secure.raw", 0);
   #endif
   for (i = 0; i < 8; ++i) {
      lu_BitstreamWrite_u32(&state, p_PokemonStorage->boxes[6][19].secure.raw[i], 32);
   }
};

void lu_WriteSaveSector_PokemonStorage04(u8* dst, const struct PokemonStorage* p_PokemonStorage) {
   u8 i, j;
   struct lu_BitstreamState state;
   lu_BitstreamInitialize(&state, dst);

   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[6][19].secure.raw", 0);
   #endif
   for (i = 8; i < 12; ++i) {
      lu_BitstreamWrite_u32(&state, p_PokemonStorage->boxes[6][19].secure.raw[i], 32);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes", 0);
   #endif
   for (j = 20; j < 30; ++j) {
      lu_BitstreamWrite_BoxPokemon(&state, &p_PokemonStorage->boxes[6][j]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes", 0);
   #endif
   for (j = 0; j < IN_BOX_COUNT; ++j) {
      lu_BitstreamWrite_BoxPokemon(&state, &p_PokemonStorage->boxes[7][j]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes", 0);
   #endif
   for (j = 0; j < 9; ++j) {
      lu_BitstreamWrite_BoxPokemon(&state, &p_PokemonStorage->boxes[8][j]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[8][9].personality", 0);
   #endif
   lu_BitstreamWrite_u32(&state, p_PokemonStorage->boxes[8][9].personality, 32);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[8][9].otId", 0);
   #endif
   lu_BitstreamWrite_u32(&state, p_PokemonStorage->boxes[8][9].otId, 32);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[8][9].nickname", 0);
   #endif
   lu_BitstreamWrite_string_optional_terminator(&state, p_PokemonStorage->boxes[8][9].nickname, POKEMON_NAME_LENGTH);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[8][9].language", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_PokemonStorage->boxes[8][9].language, 3);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[8][9].isBadEgg", 0);
   #endif
   lu_BitstreamWrite_bool(&state, p_PokemonStorage->boxes[8][9].isBadEgg);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[8][9].hasSpecies", 0);
   #endif
   lu_BitstreamWrite_bool(&state, p_PokemonStorage->boxes[8][9].hasSpecies);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[8][9].isEgg", 0);
   #endif
   lu_BitstreamWrite_bool(&state, p_PokemonStorage->boxes[8][9].isEgg);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[8][9].unused", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_PokemonStorage->boxes[8][9].unused, 5);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[8][9].otName", 0);
   #endif
   lu_BitstreamWrite_string_optional_terminator(&state, p_PokemonStorage->boxes[8][9].otName, PLAYER_NAME_LENGTH);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[8][9].markings", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_PokemonStorage->boxes[8][9].markings, 8);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[8][9].checksum", 0);
   #endif
   lu_BitstreamWrite_u16(&state, p_PokemonStorage->boxes[8][9].checksum, 16);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[8][9].unknown", 0);
   #endif
   lu_BitstreamWrite_u16(&state, p_PokemonStorage->boxes[8][9].unknown, 16);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[8][9].secure.raw", 0);
   #endif
   for (i = 0; i < 7; ++i) {
      lu_BitstreamWrite_u32(&state, p_PokemonStorage->boxes[8][9].secure.raw[i], 32);
   }
};

void lu_WriteSaveSector_PokemonStorage05(u8* dst, const struct PokemonStorage* p_PokemonStorage) {
   u8 i, j;
   struct lu_BitstreamState state;
   lu_BitstreamInitialize(&state, dst);

   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[8][9].secure.raw", 0);
   #endif
   for (i = 7; i < 12; ++i) {
      lu_BitstreamWrite_u32(&state, p_PokemonStorage->boxes[8][9].secure.raw[i], 32);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes", 0);
   #endif
   for (j = 10; j < 30; ++j) {
      lu_BitstreamWrite_BoxPokemon(&state, &p_PokemonStorage->boxes[8][j]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes", 0);
   #endif
   for (j = 0; j < 29; ++j) {
      lu_BitstreamWrite_BoxPokemon(&state, &p_PokemonStorage->boxes[9][j]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[9][29].personality", 0);
   #endif
   lu_BitstreamWrite_u32(&state, p_PokemonStorage->boxes[9][29].personality, 32);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[9][29].otId", 0);
   #endif
   lu_BitstreamWrite_u32(&state, p_PokemonStorage->boxes[9][29].otId, 32);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[9][29].nickname", 0);
   #endif
   lu_BitstreamWrite_string_optional_terminator(&state, p_PokemonStorage->boxes[9][29].nickname, POKEMON_NAME_LENGTH);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[9][29].language", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_PokemonStorage->boxes[9][29].language, 3);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[9][29].isBadEgg", 0);
   #endif
   lu_BitstreamWrite_bool(&state, p_PokemonStorage->boxes[9][29].isBadEgg);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[9][29].hasSpecies", 0);
   #endif
   lu_BitstreamWrite_bool(&state, p_PokemonStorage->boxes[9][29].hasSpecies);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[9][29].isEgg", 0);
   #endif
   lu_BitstreamWrite_bool(&state, p_PokemonStorage->boxes[9][29].isEgg);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[9][29].unused", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_PokemonStorage->boxes[9][29].unused, 5);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[9][29].otName", 0);
   #endif
   lu_BitstreamWrite_string_optional_terminator(&state, p_PokemonStorage->boxes[9][29].otName, PLAYER_NAME_LENGTH);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[9][29].markings", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_PokemonStorage->boxes[9][29].markings, 8);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[9][29].checksum", 0);
   #endif
   lu_BitstreamWrite_u16(&state, p_PokemonStorage->boxes[9][29].checksum, 16);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[9][29].unknown", 0);
   #endif
   lu_BitstreamWrite_u16(&state, p_PokemonStorage->boxes[9][29].unknown, 16);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[9][29].secure.raw", 0);
   #endif
   for (i = 0; i < 6; ++i) {
      lu_BitstreamWrite_u32(&state, p_PokemonStorage->boxes[9][29].secure.raw[i], 32);
   }
};

void lu_WriteSaveSector_PokemonStorage06(u8* dst, const struct PokemonStorage* p_PokemonStorage) {
   u8 i, j;
   struct lu_BitstreamState state;
   lu_BitstreamInitialize(&state, dst);

   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[9][29].secure.raw", 0);
   #endif
   for (i = 6; i < 12; ++i) {
      lu_BitstreamWrite_u32(&state, p_PokemonStorage->boxes[9][29].secure.raw[i], 32);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes", 0);
   #endif
   for (j = 0; j < IN_BOX_COUNT; ++j) {
      lu_BitstreamWrite_BoxPokemon(&state, &p_PokemonStorage->boxes[10][j]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes", 0);
   #endif
   for (j = 0; j < 19; ++j) {
      lu_BitstreamWrite_BoxPokemon(&state, &p_PokemonStorage->boxes[11][j]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[11][19].personality", 0);
   #endif
   lu_BitstreamWrite_u32(&state, p_PokemonStorage->boxes[11][19].personality, 32);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[11][19].otId", 0);
   #endif
   lu_BitstreamWrite_u32(&state, p_PokemonStorage->boxes[11][19].otId, 32);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[11][19].nickname", 0);
   #endif
   lu_BitstreamWrite_string_optional_terminator(&state, p_PokemonStorage->boxes[11][19].nickname, POKEMON_NAME_LENGTH);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[11][19].language", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_PokemonStorage->boxes[11][19].language, 3);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[11][19].isBadEgg", 0);
   #endif
   lu_BitstreamWrite_bool(&state, p_PokemonStorage->boxes[11][19].isBadEgg);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[11][19].hasSpecies", 0);
   #endif
   lu_BitstreamWrite_bool(&state, p_PokemonStorage->boxes[11][19].hasSpecies);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[11][19].isEgg", 0);
   #endif
   lu_BitstreamWrite_bool(&state, p_PokemonStorage->boxes[11][19].isEgg);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[11][19].unused", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_PokemonStorage->boxes[11][19].unused, 5);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[11][19].otName", 0);
   #endif
   lu_BitstreamWrite_string_optional_terminator(&state, p_PokemonStorage->boxes[11][19].otName, PLAYER_NAME_LENGTH);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[11][19].markings", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_PokemonStorage->boxes[11][19].markings, 8);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[11][19].checksum", 0);
   #endif
   lu_BitstreamWrite_u16(&state, p_PokemonStorage->boxes[11][19].checksum, 16);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[11][19].unknown", 0);
   #endif
   lu_BitstreamWrite_u16(&state, p_PokemonStorage->boxes[11][19].unknown, 16);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[11][19].secure.raw", 0);
   #endif
   for (i = 0; i < 5; ++i) {
      lu_BitstreamWrite_u32(&state, p_PokemonStorage->boxes[11][19].secure.raw[i], 32);
   }
};

void lu_WriteSaveSector_PokemonStorage07(u8* dst, const struct PokemonStorage* p_PokemonStorage) {
   u8 i, j;
   struct lu_BitstreamState state;
   lu_BitstreamInitialize(&state, dst);

   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[11][19].secure.raw", 0);
   #endif
   for (i = 5; i < 12; ++i) {
      lu_BitstreamWrite_u32(&state, p_PokemonStorage->boxes[11][19].secure.raw[i], 32);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes", 0);
   #endif
   for (j = 20; j < 30; ++j) {
      lu_BitstreamWrite_BoxPokemon(&state, &p_PokemonStorage->boxes[11][j]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes", 0);
   #endif
   for (j = 0; j < IN_BOX_COUNT; ++j) {
      lu_BitstreamWrite_BoxPokemon(&state, &p_PokemonStorage->boxes[12][j]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes", 0);
   #endif
   for (j = 0; j < 9; ++j) {
      lu_BitstreamWrite_BoxPokemon(&state, &p_PokemonStorage->boxes[13][j]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[13][9].personality", 0);
   #endif
   lu_BitstreamWrite_u32(&state, p_PokemonStorage->boxes[13][9].personality, 32);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[13][9].otId", 0);
   #endif
   lu_BitstreamWrite_u32(&state, p_PokemonStorage->boxes[13][9].otId, 32);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[13][9].nickname", 0);
   #endif
   lu_BitstreamWrite_string_optional_terminator(&state, p_PokemonStorage->boxes[13][9].nickname, POKEMON_NAME_LENGTH);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[13][9].language", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_PokemonStorage->boxes[13][9].language, 3);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[13][9].isBadEgg", 0);
   #endif
   lu_BitstreamWrite_bool(&state, p_PokemonStorage->boxes[13][9].isBadEgg);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[13][9].hasSpecies", 0);
   #endif
   lu_BitstreamWrite_bool(&state, p_PokemonStorage->boxes[13][9].hasSpecies);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[13][9].isEgg", 0);
   #endif
   lu_BitstreamWrite_bool(&state, p_PokemonStorage->boxes[13][9].isEgg);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[13][9].unused", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_PokemonStorage->boxes[13][9].unused, 5);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[13][9].otName", 0);
   #endif
   lu_BitstreamWrite_string_optional_terminator(&state, p_PokemonStorage->boxes[13][9].otName, PLAYER_NAME_LENGTH);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[13][9].markings", 0);
   #endif
   lu_BitstreamWrite_u8(&state, p_PokemonStorage->boxes[13][9].markings, 8);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[13][9].checksum", 0);
   #endif
   lu_BitstreamWrite_u16(&state, p_PokemonStorage->boxes[13][9].checksum, 16);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[13][9].unknown", 0);
   #endif
   lu_BitstreamWrite_u16(&state, p_PokemonStorage->boxes[13][9].unknown, 16);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[13][9].secure.raw", 0);
   #endif
   for (i = 0; i < 4; ++i) {
      lu_BitstreamWrite_u32(&state, p_PokemonStorage->boxes[13][9].secure.raw[i], 32);
   }
};

void lu_WriteSaveSector_PokemonStorage08(u8* dst, const struct PokemonStorage* p_PokemonStorage) {
   u8 i, j;
   struct lu_BitstreamState state;
   lu_BitstreamInitialize(&state, dst);

   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes[13][9].secure.raw", 0);
   #endif
   for (i = 4; i < 12; ++i) {
      lu_BitstreamWrite_u32(&state, p_PokemonStorage->boxes[13][9].secure.raw[i], 32);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxes", 0);
   #endif
   for (j = 10; j < 30; ++j) {
      lu_BitstreamWrite_BoxPokemon(&state, &p_PokemonStorage->boxes[13][j]);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxNames", 0);
   #endif
   for (i = 0; i < TOTAL_BOXES_COUNT; ++i) {
      lu_BitstreamWrite_string(&state, p_PokemonStorage->boxNames[i], BOX_NAME_LENGTH);
   }
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_PokemonStorage->boxWallpapers", 0);
   #endif
   for (i = 0; i < TOTAL_BOXES_COUNT; ++i) {
      lu_BitstreamWrite_u8(&state, p_PokemonStorage->boxWallpapers[i], 8);
   }
};

