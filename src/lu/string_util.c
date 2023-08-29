#include "global.h"
#include "lu/string_util.h"

#include "characters.h"

u8* lu_CopyStringWithCutoff(u8* dst, const u8* src, u8 max_length_incl_terminator) {
   u8 i;
   for(i = 0; i < max_length_incl_terminator && *src != EOS; ++i) {
      *dst = *src;
      ++dst;
      ++src;
   }
   *dst = EOS;
   return dst;
}