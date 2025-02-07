#ifndef GUARD_LU_SAVEDATA_CODEGEN_TRIGGER
#define GUARD_LU_SAVEDATA_CODEGEN_TRIGGER

void ReadSavegameSector(const void* buffer, int sectorId);
void WriteSavegameSector(void* buffer, int sectorId);

#pragma lu_bitpack generate_functions( \
   read_name = ReadSavegameSector,  \
   save_name = WriteSavegameSector, \
   data      = *gSaveBlock2Ptr | *gSaveBlock1Ptr | *gPokemonStoragePtr \
   , enable_debug_output = true \
)

//
// The plug-in should generate the following identifiers, at minimum:
//
//    void ReadSavegameSector(const void* buffer, int sectorId);
//    void WriteSavegameSector(void* buffer, int sectorId);
//
//       The generated functions for bitpacking/unpacking a sector.
//
//    const size_t __lu_bitpack_sector_count;
//
//       The number of sectors that code generation is actually using.
//

#endif