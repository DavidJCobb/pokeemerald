
//
// Include this file at the top of `save.c`, and port over the other changes 
// we made in our last bitpacking design.
//

#include "lu/bitstreams.h"
#include "lu/bitpack_options.h"

#pragma lu_bitpack enable
#pragma lu_bitpack set_options ( \
   sector_count=NUM_SECTORS_PER_SLOT, \
   sector_size=SECTOR_DATA_SIZE,  \
   bool_typename            = bool8, \
   buffer_byte_typename     = void, \
   bitstream_state_typename = lu_BitstreamState, \
   func_initialize  = lu_BitstreamInitialize, \
   func_read_bool   = lu_BitstreamRead_bool, \
   func_read_u8     = lu_BitstreamRead_u8,   \
   func_read_u16    = lu_BitstreamRead_u16,  \
   func_read_u32    = lu_BitstreamRead_u32,  \
   func_read_s8     = lu_BitstreamRead_s8,   \
   func_read_s16    = lu_BitstreamRead_s16,  \
   func_read_s32    = lu_BitstreamRead_s32,  \
   func_read_string_ut = lu_BitstreamRead_string_optional_terminator, \
   func_read_string_nt = lu_BitstreamRead_string, \
   func_read_buffer = lu_BitstreamRead_buffer, \
   func_write_bool   = lu_BitstreamWrite_bool, \
   func_write_u8     = lu_BitstreamWrite_u8,   \
   func_write_u16    = lu_BitstreamWrite_u16,  \
   func_write_u32    = lu_BitstreamWrite_u32,  \
   func_write_s8     = lu_BitstreamWrite_s8,   \
   func_write_s16    = lu_BitstreamWrite_s16,  \
   func_write_s32    = lu_BitstreamWrite_s32,  \
   func_write_string_ut = lu_BitstreamWrite_string_optional_terminator, \
   func_write_string_nt = lu_BitstreamWrite_string, \
   func_write_buffer = lu_BitstreamWrite_buffer \
)

#pragma lu_bitpack generate_functions( \
   read_name = ReadSavegameSector,  \
   save_name = WriteSavegameSector, \
   data      = *gSaveBlock2Ptr | *gSaveBlock1Ptr | *gPokemonStoragePtr \
)
//
// TODO: The above pragma is not (yet) valid. The bitpacking plug-in only supports 
// serializing variables directly, and it "thinks" in terms of declarations, not 
// expressions. These are things we'll need to fix or redesign, at least for the 
// top-level entities specified in the pragma.
//