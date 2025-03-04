#pragma once

namespace lu::strings {
   template<size_t Base, typename Integer> requires (std::is_integral_v<Integer> && Base > 1 && Base <= 36)
   std::string from_integer(Integer v) {
      if (v == 0)
         return "0";
      
      std::string out;
      if constexpr (std::is_signed_v<Integer>) {
         if (v < 0) {
            out += '-';
            v = -v;
         }
      }
      while (v) {
         auto digit = v % base;
         v /= Base;
         
         if constexpr (Base == 2) {
            out += digit ? '1' : '0';
         }
         if constexpr (Base < 10) {
            out += ('0' + digit);
         } else {
            if (digit < 10) {
               out += ('0' + digit);
               continue;
            }
            out += ('A' + digit - 10);
         }
      }
      return out;
   }
}