#include "lu/stringf.h"
#include <cstdarg>
#include <stdio.h>
#include <stdexcept>

namespace lu {
   std::string stringf(const char* format, ...) {
      va_list args1;
      va_list args2;
      va_start(args1, format);
      va_copy(args2, args1);
      
      size_t size;
      {
         int size_s = vsnprintf(nullptr, 0, format, args1) + sizeof('\0');
         if (size_s < 0) {
            va_end(args1);
            va_end(args2);
            throw std::runtime_error("lu::strings::printf_string failed");
         }
         if (size_s == 0)
            return {};
         size = (size_t)size_s;
      }
      std::string out;
      out.resize(size - sizeof('\0')); // std::string always allocates a null terminator
      vsnprintf(out.data(), size, format, args2);
      
      va_end(args1);
      va_end(args2);
      
      return out;
   }
}