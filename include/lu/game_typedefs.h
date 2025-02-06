#ifndef GUARD_LU_TEST_GAME_TYPEDEFS
#define GUARD_LU_TEST_GAME_TYPEDEFS

#include "lu/bitpack_options.h"

//
// Typedefs, to make some things (e.g. bitpacking options and stats) easier.
//

LU_BP_CATEGORY("easy-chat-word") typedef u16 EasyChatWordID;
LU_BP_CATEGORY("language") LU_BP_BITCOUNT(4) typedef u8 LanguageID;
LU_BP_CATEGORY("global-item-id") typedef u16 ItemIDGlobal;
LU_BP_CATEGORY("move-id") typedef u16 MoveID;
LU_BP_CATEGORY("player-name")  LU_BP_STRING_NT typedef u8 PlayerName[PLAYER_NAME_LENGTH + 1];
LU_BP_CATEGORY("player-name")  LU_BP_STRING_UT typedef u8 PlayerNameNoTerminator[PLAYER_NAME_LENGTH];
LU_BP_CATEGORY("pokemon-level") LU_BP_MINMAX(0,100) typedef u8 PokemonLevel;
LU_BP_CATEGORY("pokemon-name") LU_BP_STRING_NT typedef u8 PokemonName[POKEMON_NAME_LENGTH + 1];
LU_BP_CATEGORY("species-id") typedef u16 PokemonSpeciesID;

// NOTE: Trainer IDs have some other representations as well.
LU_BP_CATEGORY("trainer-id") typedef u8 TrainerID[TRAINER_ID_LENGTH];

#endif