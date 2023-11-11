#ifndef GUARD_LU_SERIALIZE_SECTOR_CharacterData
#define GUARD_LU_SERIALIZE_SECTOR_CharacterData

#include "lu/bitstreams.h"

struct SaveBlock2;
struct CustomGameOptions;
struct CustomGameSavestate;

void lu_ReadSaveSector_CharacterData00(const u8* src, struct SaveBlock2* p_SaveBlock2, struct CustomGameOptions* p_CustomGameOptions, struct CustomGameSavestate* p_CustomGameSavestate);
void lu_WriteSaveSector_CharacterData00(u8* dst, const struct SaveBlock2* p_SaveBlock2, const struct CustomGameOptions* p_CustomGameOptions, const struct CustomGameSavestate* p_CustomGameSavestate);

#endif