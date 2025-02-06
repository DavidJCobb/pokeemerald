
#include "gba/types.h" // u8 and friends
#include "lu/bitstreams.h"
#include "lu/bitpack_options.h"
#include "gba/isagbprint.h"

#define SECTOR_COUNT 4
#define SECTOR_SIZE 16

#pragma lu_bitpack enable
#pragma lu_bitpack set_options ( \
   sector_count=4, \
   sector_size=16,  \
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

// Test nested named structs.
struct Color {
   u8 r;
   u8 g;
   u8 b;
};

// Test bitpacking options on typedefs.
LU_BP_BITCOUNT(5) typedef u8 u5;

// Test stat-tracking categories and bitpacking options on typedefs.
LU_BP_CATEGORY("player-name") LU_BP_STRING_UT typedef u8 player_name [7];

typedef struct {
   u8 data;
} NamedByTypedefOnly;
typedef struct NamedByTag {
   u8 data;
} NamedByTypedef;

struct TestStruct {
   struct Color color;
   
   LU_BP_BITCOUNT(3) u8 three_bit;
   u5 five_bit;
   LU_BP_BITCOUNT(7) u8 seven_bit;
   
   struct {
      // Test multiple stat-tracking categories on a single DECL.
      //LU_BP_CATEGORY("327") LU_BP_CATEGORY("second-cat") LU_BP_MINMAX(3, 7) int three_to_seven;
      LU_BP_MINMAX(3, 7) int three_to_seven;
   };
   
   // Test stat-tracking categories on array DECLs.
   LU_BP_CATEGORY("test-names") player_name names[2];
   
   // Vary the test: make `player-name` and `test-names` stats vary from each other.
   LU_BP_DEFAULT("Carter") player_name foe_name;
   
   u8 single_element_array_1D[1];
   u8 single_element_array_2D[1][1];
   u8 single_element_array_3D[1][1][1];
   
   u5 array[3];
   
   bool8 boolean;
   
   LU_BP_AS_OPAQUE_BUFFER float decimals[3];
   
   struct {
      LU_BP_OMIT LU_BP_DEFAULT("Ana")     player_name a;
      LU_BP_OMIT LU_BP_DEFAULT("Kris")    player_name b;
      LU_BP_OMIT LU_BP_DEFAULT("Kara")    player_name c;
      LU_BP_OMIT LU_BP_DEFAULT("Hilda")   player_name d;
      LU_BP_OMIT LU_BP_DEFAULT("Janine")  player_name e;
      LU_BP_OMIT LU_BP_DEFAULT("Darlene") player_name f;
      // Test to verify that the previous name doesn't overflow:
      LU_BP_OMIT LU_BP_DEFAULT("Mary")    player_name g;
   } default_names;
   
   LU_BP_OMIT LU_BP_DEFAULT("<\"xml\">\n<>") LU_BP_STRING char xml_tricky_test[12];
   
   // Testcase: default value is string; value is not marked as a string.
   LU_BP_OMIT LU_BP_DEFAULT("<\"xml\">\n<>") char xml_tricky_test_2[12];
   
   NamedByTypedefOnly named_by_typedef_only;
   struct NamedByTag  using_tag_name;
   NamedByTypedef     using_typedef_name;
} sTestStruct;

extern void generated_read(const u8* src, int sector_id);
extern void generated_save(u8* dst, int sector_id);

#pragma lu_bitpack generate_functions( \
   read_name = generated_read,         \
   save_name = generated_save,         \
   data      = sTestStruct             \
)
//#pragma lu_bitpack debug_dump_function generated_read

#include <string.h> // memset

void print_buffer(const char* buffer, int size) {
   DebugPrintf("Buffer:\n   ");
   for(int i = 0; i < size; ++i) {
      if (i && i % 8 == 0) {
         DebugPrintf("\n   ");
      }
      DebugPrintf("%02X ", (uint8_t) buffer[i]);
   }
   DebugPrintf("\n");
}

void print_char(char c) {
   if (c >= '!' && c <= '~') {
      DebugPrintf("'%c'", (uint8_t)(c & 0xFF));
   } else {
      DebugPrintf("0x%02X", c & 0xFF);
   }
}

void print_top_level_string_member(const char* name, int size, const char* data) {
   DebugPrintf("      .%s = {\n", name);
   for(int i = 0; i < size; ++i) {
      DebugPrintf("         ");
      print_char(data[i]);
      DebugPrintf(",\n");
   }
   DebugPrintf("      },\n");
}

void print_test_struct(void) {
   DebugPrintf("sTestStruct == {\n");
   DebugPrintf("   .color == { %u, %u, %u },\n", sTestStruct.color.r, sTestStruct.color.g, sTestStruct.color.b);
   DebugPrintf("   .three_bit == %u\n", sTestStruct.three_bit);
   DebugPrintf("   .five_bit == %u\n", sTestStruct.five_bit);
   DebugPrintf("   .seven_bit == %u\n", sTestStruct.seven_bit);
   DebugPrintf("   .three_to_seven == %u\n", sTestStruct.three_to_seven);
   DebugPrintf("   .names == {\n");
   DebugPrintf("      \"%.7s\",\n", sTestStruct.names[0]);
   DebugPrintf("      \"%.7s\",\n", sTestStruct.names[1]);
   DebugPrintf("   },\n");
   DebugPrintf("   .foe_name = \"%.7s\",\n", sTestStruct.foe_name);
   DebugPrintf("   .single_element_array_1D == { %u },\n", sTestStruct.single_element_array_1D[0]);
   DebugPrintf("   .single_element_array_2D == { { %u } },\n", sTestStruct.single_element_array_2D[0][0]);
   DebugPrintf("   .single_element_array_3D == { { { %u } } },\n", sTestStruct.single_element_array_3D[0][0][0]);
   DebugPrintf("   .array == {\n");
   for(int i = 0; i < 3; ++i)
      DebugPrintf("      %u,\n", sTestStruct.array[i]);
   DebugPrintf("   },\n");
   DebugPrintf("   .boolean == %u,\n", sTestStruct.boolean);
   DebugPrintf("   .decimals == {\n");
   for(int i = 0; i < 3; ++i)
      DebugPrintf("      %f,\n", sTestStruct.decimals[i]);
   DebugPrintf("   },\n");
   DebugPrintf("   .default_names == {\n");
   DebugPrintf("      \"%.7s\",\n", sTestStruct.default_names.a);
   DebugPrintf("      \"%.7s\",\n", sTestStruct.default_names.b);
   DebugPrintf("      \"%.7s\",\n", sTestStruct.default_names.c);
   DebugPrintf("      \"%.7s\",\n", sTestStruct.default_names.d);
   DebugPrintf("      \"%.7s\",\n", sTestStruct.default_names.e);
   DebugPrintf("      \"%.7s\",\n", sTestStruct.default_names.f);
   DebugPrintf("      \"%.7s\",\n", sTestStruct.default_names.g);
   DebugPrintf("   },\n");
   DebugPrintf("   .xml_tricky_test = \"%.12s\",\n", sTestStruct.xml_tricky_test);
   DebugPrintf("   .xml_tricky_test_2 = \"%.12s\",\n", sTestStruct.xml_tricky_test_2);
   DebugPrintf("}\n");
}

extern int LuBitpack_RunTest(void) {
   u8 sector_buffers[SECTOR_COUNT][SECTOR_SIZE] = { 0 };
   
   //
   // Set up initial test data.
   //
   sTestStruct.color.r = 0xFF;
   sTestStruct.color.g = 0x74;
   sTestStruct.color.b = 0x00;
   sTestStruct.three_bit = 3;
   sTestStruct.five_bit = 1 << 4;
   sTestStruct.seven_bit = 127;
   sTestStruct.three_to_seven = 5;
   memcpy(&sTestStruct.names[0], "ABCDEFG", 7);
   memcpy(&sTestStruct.names[1], "HIJKLMN", 7);
   memcpy(&sTestStruct.foe_name, "TUVWXYZ", 7);
   sTestStruct.single_element_array_1D[0] = 1;
   sTestStruct.single_element_array_2D[0][0] = 2;
   sTestStruct.single_element_array_3D[0][0][0] = 3;
   sTestStruct.array[0] = 16;
   sTestStruct.array[1] = 17;
   sTestStruct.array[2] = 18;
   sTestStruct.boolean = 1;
   sTestStruct.decimals[0] = 1.0;
   sTestStruct.decimals[1] = 2.5;
   sTestStruct.decimals[2] = 100.0;
   memset(&sTestStruct.default_names, 0, sizeof(sTestStruct.default_names));
   memset(sTestStruct.xml_tricky_test, 0, sizeof(sTestStruct.xml_tricky_test));
   memset(sTestStruct.xml_tricky_test_2, 0, sizeof(sTestStruct.xml_tricky_test_2));
   
   const char* divider = "====================================================\n";
   
   DebugPrintf(divider);
   DebugPrintf("Test data:\n");
   DebugPrintf(divider);
   print_test_struct();
   DebugPrintf("\n");
   
   //
   // Perform save.
   //
   memset(&sector_buffers, '~', sizeof(sector_buffers));
   for(int i = 0; i < SECTOR_COUNT; ++i) {
      DebugPrintf("Saving sector %u...\n", i);
      generated_save(sector_buffers[i], i);
   }
   DebugPrintf("All sectors saved. Wiping sTestStruct...\n");
   memset(&sTestStruct, 0xCC, sizeof(sTestStruct));
   DebugPrintf("Wiped. Reviewing data...\n\n");
   
   for(int i = 0; i < SECTOR_COUNT; ++i) {
      DebugPrintf("Sector %u saved:\n", i);
      print_buffer(sector_buffers[i], sizeof(sector_buffers[i]));
      DebugPrintf("Sector %u read:\n", i);
      generated_read(sector_buffers[i], i);
      print_test_struct();
   }
   
   
   return 0;
}