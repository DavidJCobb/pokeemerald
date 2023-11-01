#include "global.h"
#include "lu/bitstreams.h"
#include "string_util.h" // for GFLib StringLength

static void _advance_position_by_bits(struct lu_BitstreamState* state, u8 bits) {
   state->shift += bits;
   while (state->shift >= 8) {
      state->shift -= 8;
      ++state->target;
   }
}
static void _advance_position_by_bytes(struct lu_BitstreamState* state, u8 bytes) {
   state->target += bytes;
}

//
// MISC:
//

void lu_BitstreamInitialize(struct lu_BitstreamState* state, u8* target) {
   state->target = target;
   state->shift  = 0;
}

//
// BITSTREAM READING:
//

static u8 _consume_byte_for_read(struct lu_BitstreamState* state, u8 bitcount, u8* consumed_bitcount) {
   u8 raw  = *state->target & (0xFF >> state->shift);
   u8 read = 8 - state->shift;
   if (bitcount < read) {
      raw = raw >> (read - bitcount);
      _advance_position_by_bits(state, bitcount);
      *consumed_bitcount = bitcount;
   } else {
      _advance_position_by_bits(state, read);
      *consumed_bitcount = read;
   }
   return raw;
}

u8 lu_BitstreamRead_bool(struct lu_BitstreamState* state) {
   u8 consumed;
   return _consume_byte_for_read(state, 1, &consumed) & 1;
}

u8 lu_BitstreamRead_u8(struct lu_BitstreamState* state, u8 bitcount) {
   u8 result;
   u8 raw;
   u8 consumed;
   u8 remaining;
   raw = _consume_byte_for_read(state, bitcount, &consumed);
   result    = raw;
   remaining = bitcount - consumed;
   while (remaining) {
      raw   = _consume_byte_for_read(state, remaining, &consumed);
      result = (result << consumed) | raw;
      remaining -= consumed;
   }
   return result;
}
u16 lu_BitstreamRead_u16(struct lu_BitstreamState* state, u8 bitcount) {
   u16 result;
   u8  raw;
   u8  consumed;
   u8  remaining;
   raw = _consume_byte_for_read(state, bitcount, &consumed);
   result    = raw;
   remaining = bitcount - consumed;
   while (remaining) {
      raw   = _consume_byte_for_read(state, remaining, &consumed);
      result = (result << consumed) | raw;
      remaining -= consumed;
   }
   return result;
}
u32 lu_BitstreamRead_u32(struct lu_BitstreamState* state, u8 bitcount) {
   u32 result;
   u8  raw;
   u8  consumed;
   u8  remaining;
   raw = _consume_byte_for_read(state, bitcount, &consumed);
   result    = raw;
   remaining = bitcount - consumed;
   while (remaining) {
      raw   = _consume_byte_for_read(state, remaining, &consumed);
      result = (result << consumed) | raw;
      remaining -= consumed;
   }
   return result;
}

s8 lu_BitstreamRead_s8(struct lu_BitstreamState* state, u8 bitcount) {
   s8 result = (s8) lu_BitstreamRead_u8(state, bitcount);
   if (bitcount < 8) {
      u8 sign_bit = result >> (bitcount - 1);
      if (sign_bit) {
         // Set all bits "above" the serialized ones, to sign-extend.
         result |= ~(((u8)1 << bitcount) - 1);
      }
   }
   return result;
}
s16 lu_BitstreamRead_s16(struct lu_BitstreamState* state, u8 bitcount) {
   s16 result = (s16) lu_BitstreamRead_u16(state, bitcount);
   if (bitcount < 16) {
      u8 sign_bit = result >> (bitcount - 1);
      if (sign_bit) {
         // Set all bits "above" the serialized ones, to sign-extend.
         result |= ~(((u16)1 << bitcount) - 1);
      }
   }
   return result;
}
s32 lu_BitstreamRead_s32(struct lu_BitstreamState* state, u8 bitcount) {
   s16 result = (s32) lu_BitstreamRead_u32(state, bitcount);
   if (bitcount < 32) {
      u8 sign_bit = result >> (bitcount - 1);
      if (sign_bit) {
         // Set all bits "above" the serialized ones, to sign-extend.
         result |= ~(((u16)1 << bitcount) - 1);
      }
   }
   return result;
}

void lu_BitstreamRead_string(struct lu_BitstreamState* state, u8* dst, u16 max_length, u8 length_bitcount) {
   u16 i;
   u16 len;
   
   for(i = 0; i < max_length; ++i)
      dst[i] = 0xFF; // EOS (TODO: Does Game Freak prefer EOS or 0? Do they ever checksum a string?)
   
   len = lu_BitstreamRead_u16(state, length_bitcount);
   for(i = 0; i < len; ++i)
      dst[i] = lu_BitstreamRead_u8(state, 8);
   dst[max_length] = 0xFF; // EOS (TODO: Does Game Freak prefer EOS or 0? Do they ever checksum a string?)
}
void lu_BitstreamRead_string_optional_terminator(struct lu_BitstreamState* state, u8* dst, u16 max_length) {
   u16 i;
   for(i = 0; i < max_length; ++i)
      dst[i] = lu_BitstreamRead_u8(state, 8);
}

void lu_BitstreamRead_buffer(struct lu_BitstreamState* state, void* value, u16 bytecount) {
   u16 i;
   for(i = 0; i < bytecount; ++i)
      *((u8*)value + i) = lu_BitstreamRead_u8(state, 8);
}

//
// BITSTREAM WRITING:
//

void lu_BitstreamWrite_bool(struct lu_BitstreamState* state, bool8 value) {
   if (state->shift == 0) {
      *state->target = value ? 0x80 : 0x00;
      ++state->shift;
      return;
   }
   {
      u8 mask = 1 << (8 - state->shift - 1);
      if (value) {
         *state->target |= mask;
      } else {
         *state->target &= ~mask;
      }
   }
   ++state->shift;
   if (state->shift == 8) {
      state->shift = 0;
      ++state->target;
   }
}
void lu_BitstreamWrite_u8(struct lu_BitstreamState* state, u8 value, u8 bitcount) {
   if (bitcount < 8) {
      value &= ((u8)1 << bitcount) - 1;
   }
   
   while (bitcount > 0) {
      u8 extra;
      
      if (state->shift == 0) {
         if (bitcount < 8) {
            *state->target = value << (8 - bitcount);
            _advance_position_by_bits(state, bitcount);
            return;
         }
         *state->target = value >> (bitcount - 8);
         _advance_position_by_bytes(state, 1);
         bitcount -= 8;
         continue;
      }
      
      extra = 8 - state->shift;
      if (bitcount <= extra) {
         // Value can fit entirely within the current byte.
         *state->target |= value << (extra - bitcount);
         state->shift += bitcount;
         return;
      }
      
      *state->target &= ~((u8)0xFF >> state->shift); // clear the bits we're about to write
      *state->target |= ((value >> (bitcount - extra)) & 0xFF);
      _advance_position_by_bits(state, extra);
      bitcount -= extra;
   }
}

void lu_BitstreamWrite_u16(struct lu_BitstreamState* state, u16 value, u8 bitcount) {
   if (bitcount < 16) {
      value &= ((u16)1 << bitcount) - 1;
   }
   
   while (bitcount > 0) {
      u8 extra;
      
      if (state->shift == 0) {
         /*//
         if (bitcount < 8) {
            *state->target = value << (8 - bitcount);
            _advance_position_by_bits(state, bitcount);
            return;
         }
         //*/
         *state->target = value >> (bitcount - 8);
         _advance_position_by_bytes(state, 1);
         bitcount -= 8;
         continue;
      }
      
      extra = 8 - state->shift;
      if (bitcount <= extra) {
         // Value can fit entirely within the current byte.
         *state->target |= value << (extra - bitcount);
         state->shift += bitcount;
         return;
      }
      
      *state->target &= ~((u8)0xFF >> state->shift); // clear the bits we're about to write
      *state->target |= ((value >> (bitcount - extra)) & 0xFF);
      _advance_position_by_bits(state, extra);
      bitcount -= extra;
   }
}

void lu_BitstreamWrite_u32(struct lu_BitstreamState* state, u32 value, u8 bitcount) {
   if (bitcount < 32) {
      value &= ((u32)1 << bitcount) - 1;
   }
   
   while (bitcount > 0) {
      u8 extra;
      
      if (state->shift == 0) {
         /*//
         if (bitcount < 8) {
            *state->target = value << (8 - bitcount);
            _advance_position_by_bits(state, bitcount);
            return;
         }
         //*/
         *state->target = value >> (bitcount - 8);
         _advance_position_by_bytes(state, 1);
         bitcount -= 8;
         continue;
      }
      
      extra = 8 - state->shift;
      if (bitcount <= extra) {
         // Value can fit entirely within the current byte.
         *state->target |= value << (extra - bitcount);
         state->shift += bitcount;
         return;
      }
      
      *state->target &= ~((u8)0xFF >> state->shift); // clear the bits we're about to write
      *state->target |= ((value >> (bitcount - extra)) & 0xFF);
      _advance_position_by_bits(state, extra);
      bitcount -= extra;
   }
}

void lu_BitstreamWrite_string(struct lu_BitstreamState* state, u8* value, u16 max_length, u8 length_bitcount) {
   u8  length_bitcount;
   u16 len = StringLength(value);
   u16 i;
   
   if (length_bitcount <= 8) {
      lu_BitstreamWrite_u8(state, len, length_bitcount);
   } else {
      lu_BitstreamWrite_u16(state, len, length_bitcount);
   }
   
   for(i = 0; i < len; ++i) {
      lu_BitstreamWrite_u8(state, value[i], 8);
   }
   for(; i < max_length; ++i) {
      lu_BitstreamWrite_u8(state, 0xFF, 8); // EOS
   }
}
void lu_BitstreamWrite_string_optional_terminator(struct lu_BitstreamState* state, u8* value, u16 max_length) {
   u16 i;
   for(i = 0; i < max_length; ++i)
      lu_BitstreamWrite_u8(state, value[i], 8);
}

void lu_BitstreamWrite_buffer(struct lu_BitstreamState* state, void* value, u16 bytecount) {
   u16 i;
   for(i = 0; i < bytecount; ++i)
      lu_BitstreamWrite_u8(state, *((u8*)value + i), 8);
}
