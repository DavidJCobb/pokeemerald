#include "lu/generated/sector-serialize/CharacterData.h"

#include "global.h"
// whole-struct serialize funcs:
#include "lu/generated/struct-serialize/serialize_SaveBlock2.h"
#include "lu/generated/struct-serialize/serialize_CustomGameOptions.h"
#include "lu/generated/struct-serialize/serialize_CustomGameSavestate.h"

void lu_ReadSaveSector_CharacterData00(const u8* src, struct SaveBlock2* p_SaveBlock2, struct CustomGameOptions* p_CustomGameOptions, struct CustomGameSavestate* p_CustomGameSavestate) {
   struct lu_BitstreamState state;
   lu_BitstreamInitialize(&state, (u8*)src); // need to cast away constness to store it here

   lu_BitstreamRead_SaveBlock2(&state, p_SaveBlock2);
   lu_BitstreamRead_CustomGameOptions(&state, p_CustomGameOptions);
   lu_BitstreamRead_CustomGameSavestate(&state, p_CustomGameSavestate);
};

void lu_WriteSaveSector_CharacterData00(u8* dst, const struct SaveBlock2* p_SaveBlock2, const struct CustomGameOptions* p_CustomGameOptions, const struct CustomGameSavestate* p_CustomGameSavestate) {
   struct lu_BitstreamState state;
   lu_BitstreamInitialize(&state, dst);

   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock2", 0);
   #endif
   lu_BitstreamWrite_SaveBlock2(&state, p_SaveBlock2);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_CustomGameOptions", 0);
   #endif
   lu_BitstreamWrite_CustomGameOptions(&state, p_CustomGameOptions);
   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_CustomGameSavestate", 0);
   #endif
   lu_BitstreamWrite_CustomGameSavestate(&state, p_CustomGameSavestate);
};

