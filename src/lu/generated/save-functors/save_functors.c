#include "global.h"

// sector-group serialize functions: 
#include "lu/generated/sector-serialize/CharacterData.h"
#include "lu/generated/sector-serialize/WorldData.h"
#include "lu/generated/sector-serialize/PokemonStorage.h"

// globals to serialize:
#include "lu/custom_game_options.h"
#include "pokemon_storage_system.h"

extern bool8 ReadSector_CharacterData(u8 sliceNum, const u8* src) {
   if (sliceNum == 0) {
      lu_ReadSaveSector_CharacterData00(src, gSaveBlock2Ptr, &gCustomGameOptions, &gCustomGameSavestate);
   } else {
      return FALSE;
   }
   return TRUE;
}
extern bool8 WriteSector_CharacterData(u8 sliceNum, u8* dst) {
   if (sliceNum == 0) {
      lu_WriteSaveSector_CharacterData00(dst, gSaveBlock2Ptr, &gCustomGameOptions, &gCustomGameSavestate);
   } else {
      return FALSE;
   }
   return TRUE;
}

extern bool8 ReadSector_WorldData(u8 sliceNum, const u8* src) {
   if (sliceNum == 0) {
      lu_ReadSaveSector_WorldData00(src, gSaveBlock1Ptr);
   } else if (sliceNum == 1) {
      lu_ReadSaveSector_WorldData01(src, gSaveBlock1Ptr);
   } else if (sliceNum == 2) {
      lu_ReadSaveSector_WorldData02(src, gSaveBlock1Ptr);
   } else if (sliceNum == 3) {
      lu_ReadSaveSector_WorldData03(src, gSaveBlock1Ptr);
   } else {
      return FALSE;
   }
   return TRUE;
}
extern bool8 WriteSector_WorldData(u8 sliceNum, u8* dst) {
   if (sliceNum == 0) {
      lu_WriteSaveSector_WorldData00(dst, gSaveBlock1Ptr);
   } else if (sliceNum == 1) {
      lu_WriteSaveSector_WorldData01(dst, gSaveBlock1Ptr);
   } else if (sliceNum == 2) {
      lu_WriteSaveSector_WorldData02(dst, gSaveBlock1Ptr);
   } else if (sliceNum == 3) {
      lu_WriteSaveSector_WorldData03(dst, gSaveBlock1Ptr);
   } else {
      return FALSE;
   }
   return TRUE;
}

extern bool8 ReadSector_PokemonStorage(u8 sliceNum, const u8* src) {
   if (sliceNum == 0) {
      lu_ReadSaveSector_PokemonStorage00(src, gPokemonStoragePtr);
   } else if (sliceNum == 1) {
      lu_ReadSaveSector_PokemonStorage01(src, gPokemonStoragePtr);
   } else if (sliceNum == 2) {
      lu_ReadSaveSector_PokemonStorage02(src, gPokemonStoragePtr);
   } else if (sliceNum == 3) {
      lu_ReadSaveSector_PokemonStorage03(src, gPokemonStoragePtr);
   } else if (sliceNum == 4) {
      lu_ReadSaveSector_PokemonStorage04(src, gPokemonStoragePtr);
   } else if (sliceNum == 5) {
      lu_ReadSaveSector_PokemonStorage05(src, gPokemonStoragePtr);
   } else if (sliceNum == 6) {
      lu_ReadSaveSector_PokemonStorage06(src, gPokemonStoragePtr);
   } else if (sliceNum == 7) {
      lu_ReadSaveSector_PokemonStorage07(src, gPokemonStoragePtr);
   } else if (sliceNum == 8) {
      lu_ReadSaveSector_PokemonStorage08(src, gPokemonStoragePtr);
   } else {
      return FALSE;
   }
   return TRUE;
}
extern bool8 WriteSector_PokemonStorage(u8 sliceNum, u8* dst) {
   if (sliceNum == 0) {
      lu_WriteSaveSector_PokemonStorage00(dst, gPokemonStoragePtr);
   } else if (sliceNum == 1) {
      lu_WriteSaveSector_PokemonStorage01(dst, gPokemonStoragePtr);
   } else if (sliceNum == 2) {
      lu_WriteSaveSector_PokemonStorage02(dst, gPokemonStoragePtr);
   } else if (sliceNum == 3) {
      lu_WriteSaveSector_PokemonStorage03(dst, gPokemonStoragePtr);
   } else if (sliceNum == 4) {
      lu_WriteSaveSector_PokemonStorage04(dst, gPokemonStoragePtr);
   } else if (sliceNum == 5) {
      lu_WriteSaveSector_PokemonStorage05(dst, gPokemonStoragePtr);
   } else if (sliceNum == 6) {
      lu_WriteSaveSector_PokemonStorage06(dst, gPokemonStoragePtr);
   } else if (sliceNum == 7) {
      lu_WriteSaveSector_PokemonStorage07(dst, gPokemonStoragePtr);
   } else if (sliceNum == 8) {
      lu_WriteSaveSector_PokemonStorage08(dst, gPokemonStoragePtr);
   } else {
      return FALSE;
   }
   return TRUE;
}

