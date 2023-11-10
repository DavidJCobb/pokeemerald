#ifndef GUARD_LU_SERIALIZE_SECTOR_PokemonStorage
#define GUARD_LU_SERIALIZE_SECTOR_PokemonStorage

#include "lu/bitstreams.h"

struct PokemonStorage;

void lu_ReadSaveSector_PokemonStorage00(const u8* src, struct PokemonStorage* p_PokemonStorage);
void lu_WriteSaveSector_PokemonStorage00(u8* dst, const struct PokemonStorage* p_PokemonStorage);
void lu_ReadSaveSector_PokemonStorage01(const u8* src, struct PokemonStorage* p_PokemonStorage);
void lu_WriteSaveSector_PokemonStorage01(u8* dst, const struct PokemonStorage* p_PokemonStorage);
void lu_ReadSaveSector_PokemonStorage02(const u8* src, struct PokemonStorage* p_PokemonStorage);
void lu_WriteSaveSector_PokemonStorage02(u8* dst, const struct PokemonStorage* p_PokemonStorage);
void lu_ReadSaveSector_PokemonStorage03(const u8* src, struct PokemonStorage* p_PokemonStorage);
void lu_WriteSaveSector_PokemonStorage03(u8* dst, const struct PokemonStorage* p_PokemonStorage);
void lu_ReadSaveSector_PokemonStorage04(const u8* src, struct PokemonStorage* p_PokemonStorage);
void lu_WriteSaveSector_PokemonStorage04(u8* dst, const struct PokemonStorage* p_PokemonStorage);
void lu_ReadSaveSector_PokemonStorage05(const u8* src, struct PokemonStorage* p_PokemonStorage);
void lu_WriteSaveSector_PokemonStorage05(u8* dst, const struct PokemonStorage* p_PokemonStorage);
void lu_ReadSaveSector_PokemonStorage06(const u8* src, struct PokemonStorage* p_PokemonStorage);
void lu_WriteSaveSector_PokemonStorage06(u8* dst, const struct PokemonStorage* p_PokemonStorage);
void lu_ReadSaveSector_PokemonStorage07(const u8* src, struct PokemonStorage* p_PokemonStorage);
void lu_WriteSaveSector_PokemonStorage07(u8* dst, const struct PokemonStorage* p_PokemonStorage);
void lu_ReadSaveSector_PokemonStorage08(const u8* src, struct PokemonStorage* p_PokemonStorage);
void lu_WriteSaveSector_PokemonStorage08(u8* dst, const struct PokemonStorage* p_PokemonStorage);

#endif