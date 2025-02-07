
#define SAVEDATA_SERIALIZATION_VERSION 1

#include "lu/bitstreams.h"
#include "lu/bitpack_options.h"

#include "save.h" // sector-related macros
//
// NOTE: Due to an as-yet-undiagnosted bug in the bitpack plug-in or 
//       in GCC, we have to use macro values directly. The sector 
//       count should be NUM_SECTORS_PER_SLOT and the sector size 
//       should be SECTOR_DATA_SIZE. For some reason, GCC isn't 
//       expanding macros within the pragma, which is especially 
//       baffling because... well, it does in all of the plug-in's 
//       testcase builds.

#pragma lu_bitpack set_options ( \
   sector_count= 14 , \
   sector_size= 3968 ,  \
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
