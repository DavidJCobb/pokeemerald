#include "lu/generated/sector-serialize/CharacterData.h"

#include "global.h"
// whole-struct serialize funcs:
#include "lu/generated/struct-serialize/serialize_SaveBlock2.h"

void lu_ReadSaveSector_CharacterData00(const u8* src, struct SaveBlock2* p_SaveBlock2) {
   struct lu_BitstreamState state;
   lu_BitstreamInitialize(&state, (u8*)src); // need to cast away constness to store it here

   lu_BitstreamRead_SaveBlock2(&state, p_SaveBlock2);
};

void lu_WriteSaveSector_CharacterData00(u8* dst, const struct SaveBlock2* p_SaveBlock2) {
   struct lu_BitstreamState state;
   lu_BitstreamInitialize(&state, dst);

   #ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE
      DebugPrintf("Writing field: p_SaveBlock2", 0);
   #endif
   lu_BitstreamWrite_SaveBlock2(&state, p_SaveBlock2);
};

