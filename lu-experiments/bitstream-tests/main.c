#include "bitstreams.h"

int main() {
   u8 buffer[20];

   struct lu_BitstreamState state;
   lu_BitstreamInitialize(&state, buffer);
   lu_BitstreamWrite_bool(&state, 1);
   lu_BitstreamWrite_bool(&state, 0);
   lu_BitstreamWrite_bool(&state, 1);
   lu_BitstreamWrite_bool(&state, 1);
   lu_BitstreamWrite_bool(&state, 0);
   lu_BitstreamWrite_u8(&state, 5, 3);
   __debugbreak();

   {  // Test read/write bool
      u8 valid_bytes[8] = { 0b10000000, 0b01000000, 0b00100000, 0b00010000, 0b1000, 0b100, 0b10, 0b1 };
      u8 offset;

      for (offset = 0; offset < 8; ++offset) {
         u8 value;
         for (value = 0; value < 2; ++value) {
            u8 skip;
            u8 test;

            lu_BitstreamInitialize(&state, buffer);
            for (skip = 0; skip < offset; ++skip) {
               lu_BitstreamWrite_bool(&state, 0);
            }
            lu_BitstreamWrite_bool(&state, value);

            if (value == 1) {
               if (buffer[0] != valid_bytes[offset])
                  __debugbreak();
            }

            lu_BitstreamInitialize(&state, buffer);
            for (skip = 0; skip < offset; ++skip) {
               bool16 v = lu_BitstreamRead_bool(&state);
               if (v)
                  __debugbreak(); // padding bit is actually 1
            }
            test = lu_BitstreamRead_bool(&state, 8);
            if (test != value)
               __debugbreak(); // value is incorrect
         }
      }
   }

   {  // Test read/write u8
      u8 offset;
      for (offset = 0; offset < 8; ++offset) {
         u16 value;
         for (value = 0; value < (1 << 8); ++value) {
            u8 skip;
            u8 test;

            lu_BitstreamInitialize(&state, buffer);
            for (skip = 0; skip < offset; ++skip) {
               lu_BitstreamWrite_bool(&state, 0);
            }
            lu_BitstreamWrite_u8(&state, (u8)value, 8);

            lu_BitstreamInitialize(&state, buffer);
            for (skip = 0; skip < offset; ++skip) {
               bool16 v = lu_BitstreamRead_bool(&state);
               if (v)
                  __debugbreak(); // padding bit is actually 1
            }
            test = lu_BitstreamRead_u8(&state, 8);
            if (test != value)
               __debugbreak(); // value is incorrect
         }
      }
   }

   return 0;
};