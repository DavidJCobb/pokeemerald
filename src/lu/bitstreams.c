#include "gba/types.h" // u8 and friends
#include "constants/characters.h" // EOS
#include "lu/bitstreams.h"

static void _advance_position_by_bits(struct lu_BitstreamState* state, u8 bits) {
   state->shift += bits;
   while (state->shift >= 8) {
      state->target += state->shift / 8;
      #ifndef NDEBUG
         state->size += state->shift / 8;
      #endif
      state->shift %= 8;
   }
}
static void _advance_position_by_bytes(struct lu_BitstreamState* state, u8 bytes) {
   state->target += bytes;
   #ifndef NDEBUG
      state->size += bytes;
   #endif
}

#ifndef NDEBUG
   #define INVOKE_POST_WRITE_HANDLERS 1
#endif

static void _post_write_integral(struct lu_BitstreamState* state, u8 bits_written, u32 value_written) {
}
static void _post_write_buffer(struct lu_BitstreamState* state, u8 string_length) {
}
static void _post_write_string(struct lu_BitstreamState* state, u8 string_length) {
}

//
// MISC:
//

void lu_BitstreamInitialize(struct lu_BitstreamState* state, u8* target) {
   state->target = target;
   state->shift  = 0;
   #ifndef NDEBUG
      state->size = 0;
   #endif
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
      u8 sign_bit = (u8)result >> (bitcount - 1); // cast to avoid sign-extension
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
      u8 sign_bit = (u16)result >> (bitcount - 1); // cast to avoid sign-extension
      if (sign_bit) {
         // Set all bits "above" the serialized ones, to sign-extend.
         result |= ~(((u16)1 << bitcount) - 1);
      }
   }
   return result;
}
s32 lu_BitstreamRead_s32(struct lu_BitstreamState* state, u8 bitcount) {
   s32 result = (s32) lu_BitstreamRead_u32(state, bitcount);
   if (bitcount < 32) {
      u8 sign_bit = (u32)result >> (bitcount - 1); // cast to avoid sign-extension
      if (sign_bit) {
         // Set all bits "above" the serialized ones, to sign-extend.
         result |= ~(((u32)1 << bitcount) - 1);
      }
   }
   return result;
}

void lu_BitstreamRead_string(struct lu_BitstreamState* state, u8* dst, u16 max_length) {
   u16   i;
   bool8 seen_end;
   u16 len;
   
   seen_end = 0;
   for(i = 0; i < max_length; ++i) {
      dst[i] = lu_BitstreamRead_u8(state, 8);
      if (dst[i] == EOS) {
         seen_end = 1;
      } else if (seen_end) {
         dst[i] = EOS;
      }
   }
   dst[max_length] = EOS;
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
      #ifdef INVOKE_POST_WRITE_HANDLERS
         _post_write_integral(state, 1, value);
      #endif
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
   #ifdef INVOKE_POST_WRITE_HANDLERS
      _post_write_integral(state, 1, value);
   #endif
}
void lu_BitstreamWrite_u8(struct lu_BitstreamState* state, u8 value, u8 bitcount) {
   #ifdef INVOKE_POST_WRITE_HANDLERS
      u8 original_bitcount = bitcount;
   #endif
   
   if (bitcount < 8) {
      value &= ((u8)1 << bitcount) - 1;
   }
   
   while (bitcount > 0) {
      u8 extra;
      
      if (state->shift == 0) {
         if (bitcount < 8) {
            *state->target = value << (8 - bitcount);
            _advance_position_by_bits(state, bitcount);
            #ifdef INVOKE_POST_WRITE_HANDLERS
               _post_write_integral(state, original_bitcount, value);
            #endif
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
         _advance_position_by_bits(state, bitcount);
         #ifdef INVOKE_POST_WRITE_HANDLERS
            _post_write_integral(state, original_bitcount, value);
         #endif
         return;
      }
      
      *state->target &= ~((u8)0xFF >> state->shift); // clear the bits we're about to write
      *state->target |= ((value >> (bitcount - extra)) & 0xFF);
      _advance_position_by_bits(state, extra);
      bitcount -= extra;
   }
   #ifdef INVOKE_POST_WRITE_HANDLERS
      _post_write_integral(state, original_bitcount, value);
   #endif
}

void lu_BitstreamWrite_u16(struct lu_BitstreamState* state, u16 value, u8 bitcount) {
   #ifdef INVOKE_POST_WRITE_HANDLERS
      u8 original_bitcount = bitcount;
   #endif
   
   if (bitcount < 16) {
      value &= ((u16)1 << bitcount) - 1;
   }
   
   while (bitcount > 0) {
      u8 extra;
      
      if (state->shift == 0) {
         if (bitcount < 8) {
            *state->target = (u8)value << (8 - bitcount);
            _advance_position_by_bits(state, bitcount);
            #ifdef INVOKE_POST_WRITE_HANDLERS
               _post_write_integral(state, original_bitcount, value);
            #endif
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
         _advance_position_by_bits(state, bitcount);
         #ifdef INVOKE_POST_WRITE_HANDLERS
            _post_write_integral(state, original_bitcount, value);
         #endif
         return;
      }
      
      *state->target &= ~((u8)0xFF >> state->shift); // clear the bits we're about to write
      *state->target |= ((value >> (bitcount - extra)) & 0xFF);
      _advance_position_by_bits(state, extra);
      bitcount -= extra;
   }
   #ifdef INVOKE_POST_WRITE_HANDLERS
      _post_write_integral(state, original_bitcount, value);
   #endif
}

void lu_BitstreamWrite_u32(struct lu_BitstreamState* state, u32 value, u8 bitcount) {
   #ifdef INVOKE_POST_WRITE_HANDLERS
      u8 original_bitcount = bitcount;
   #endif
   
   if (bitcount < 32) {
      value &= ((u32)1 << bitcount) - 1;
   }
   
   while (bitcount > 0) {
      u8 extra;
      
      if (state->shift == 0) {
         if (bitcount < 8) {
            *state->target = (u8)value << (8 - bitcount);
            _advance_position_by_bits(state, bitcount);
            #ifdef INVOKE_POST_WRITE_HANDLERS
               _post_write_integral(state, original_bitcount, value);
            #endif
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
         _advance_position_by_bits(state, bitcount);
         #ifdef INVOKE_POST_WRITE_HANDLERS
            _post_write_integral(state, original_bitcount, value);
         #endif
         return;
      }
      
      *state->target &= ~((u8)0xFF >> state->shift); // clear the bits we're about to write
      *state->target |= ((value >> (bitcount - extra)) & 0xFF);
      _advance_position_by_bits(state, extra);
      bitcount -= extra;
   }
   #ifdef INVOKE_POST_WRITE_HANDLERS
      _post_write_integral(state, original_bitcount, value);
   #endif
}

void lu_BitstreamWrite_s8(struct lu_BitstreamState* state, s8 value, u8 bitcount) {
   lu_BitstreamWrite_u8(state, (u8)value, bitcount);
}
void lu_BitstreamWrite_s16(struct lu_BitstreamState* state, s16 value, u8 bitcount) {
   lu_BitstreamWrite_u16(state, (u16)value, bitcount);
}
void lu_BitstreamWrite_s32(struct lu_BitstreamState* state, s32 value, u8 bitcount) {
   lu_BitstreamWrite_u32(state, (u32)value, bitcount);
}


void lu_BitstreamWrite_string(struct lu_BitstreamState* state, const u8* value, u16 max_length) {
   u16 i;
   u16 len;
   
   len = max_length;
   for(i = 0; i < max_length; ++i) {
      if (value[i] == EOS) {
         len = i;
         break;
      }
   }
   
   for(i = 0; i < len; ++i) {
      lu_BitstreamWrite_u8(state, value[i], 8);
   }
   for(; i < max_length; ++i) {
      lu_BitstreamWrite_u8(state, EOS, 8);
   }
   #ifdef INVOKE_POST_WRITE_HANDLERS
      _post_write_string(state, max_length);
   #endif
}
void lu_BitstreamWrite_string_optional_terminator(struct lu_BitstreamState* state, const u8* value, u16 max_length) {
   u16 i;
   for(i = 0; i < max_length; ++i)
      lu_BitstreamWrite_u8(state, value[i], 8);
   //
   #ifdef INVOKE_POST_WRITE_HANDLERS
      _post_write_string(state, max_length);
   #endif
}

void lu_BitstreamWrite_buffer(struct lu_BitstreamState* state, const void* value, u16 bytecount) {
   u16 i;
   for(i = 0; i < bytecount; ++i)
      lu_BitstreamWrite_u8(state, *((const u8*)value + i), 8);
   //
   #ifdef INVOKE_POST_WRITE_HANDLERS
      _post_write_buffer(state, bytecount);
   #endif
}
