#include "global.h"

extern bool8 ReadSector_CharacterData(u8 sliceNum, const u8* src);
extern bool8 WriteSector_CharacterData(u8 sliceNum, u8* dst);
extern bool8 ReadSector_WorldData(u8 sliceNum, const u8* src);
extern bool8 WriteSector_WorldData(u8 sliceNum, u8* dst);
extern bool8 ReadSector_PokemonStorage(u8 sliceNum, const u8* src);
extern bool8 WriteSector_PokemonStorage(u8 sliceNum, u8* dst);
