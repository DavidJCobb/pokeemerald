#include "global.h"
#include "lu/bitstreams.h"

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

u8 lu_BitstreamRead_bool(struct lu_BitstreamState* state, u8 bitcount) {
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
   s16 result = (s16) lu_BitstreamRead_u8(state, bitcount);
   if (bitcount < 16) {
      u8 sign_bit = result >> (bitcount - 1);
      if (sign_bit) {
         // Set all bits "above" the serialized ones, to sign-extend.
         result |= ~(((u16)1 << bitcount) - 1);
      }
   }
   return result;
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
   u8 mask = 1 << (8 - state->shift - 1);
   if (value) {
      *state->target |= mask;
   } else {
      *state->target &= ~mask;
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
      
      u8 extra = 8 - state->shift;
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
      
      u8 extra = 8 - state->shift;
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