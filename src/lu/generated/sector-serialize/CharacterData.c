#include "lu/generated/sector-serialize/CharacterData.h"

// whole-struct serialize funcs:
#include "lu/generated/struct-serialize/serialize_SaveBlock2.h"

void lu_ReadSaveSector_CharacterData00(const u8* src, struct SaveBlock2* p_SaveBlock2) {
   struct lu_BitstreamState state;
   state.target = (u8*)src;
   state.shift  = 0;
   lu_BitstreamRead_SaveBlock2(&state, p_SaveBlock2);
};

void lu_WriteSaveSector_CharacterData00(u8* dst, const struct SaveBlock2* p_SaveBlock2) {
   struct lu_BitstreamState state;
   state.target = dst;
   state.shift  = 0;
   lu_BitstreamWrite_SaveBlock2(&state, p_SaveBlock2);
};

