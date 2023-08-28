#pragma once
#include <string.h>
#include "../common/types.h"

#include "string_wrap.h"

const char* test_string = "This is a test string with no line breaks. Where will it be wrapped? I suppose we'll find out.\xFF";

// ugh
static void _to_game_freak_string(u8* str) {
   while (TRUE) {
      u8 c = *str;
      if (c == (u8)'\0') {
         *str = 0xFF;
         break;
      }
      if (c == (u8)'\xFF') {
         break;
      }
      if (c == '\n') {
         *str = (u8)'\xFE';
      } else if (c == ' ') {
         *str = '\0';
      } else if (c >= 'a' && c <= 'z') {
         c -= 'a';
         c += 0xD5;
         *str = c;
         continue;
      } else if (c >= 'A' && c <= 'Z') {
         c -= 'A';
         c += 0xBB;
         *str = c;
         continue;
      } else if (c == '.') {
         *str = (u8)0xAD;
      } else if (c == '?') {
         *str = (u8)0xAC;
      } else if (c == '\'') {
         *str = (u8)0xB4;
      }
      ++str;
   }
}

int main() {
   u8 buffer[0x512];

   int size = strlen(test_string);

   memcpy(buffer, test_string, size);
   _to_game_freak_string(buffer);

   // AddTextPrinterForMessage doesn't return the printer. Printers are stack-allocated 
   // and I guess the internal funcs just copy them.
   //
   // We need a function or set of macros, then, that encapsulates the default printer 
   // configuration for messages.

   lu_PrepStringWrap_Normal();
   lu_StringWrap(buffer);

   // GF encoding back to ASCII:
   for (int i = 0; i < size; ++i) {
      u8 c = buffer[i];
      if (c == (u8)'\xFA') {
         buffer[i] = '\v';
      } else if (c == (u8)'\xFE') {
         buffer[i] = '\n';
      } else if (c == (u8)'\xFF') {
         buffer[i] = '\0';
      } else if (c == (u8)'\0') {
         buffer[i] = ' ';
      } else {
         buffer[i] = test_string[i];
      }
   }

   // Let me inspect it in the debugger:
   __debugbreak();

   return 0;
}