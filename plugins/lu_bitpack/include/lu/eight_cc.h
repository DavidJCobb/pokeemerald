#pragma once
#include <cstdint>
#include <type_traits>

namespace lu {
   struct eight_cc {
      public:
         union {
            uint64_t value = 0;
            char     bytes[8];
         };
         
         constexpr eight_cc() {}
         constexpr eight_cc(const char* s) {
            this->value = 0;
            if (!s)
               return;
            uint8_t i = 0;
            for (; i < 8; ++i) {
               if (!s[i])
                  break;
               this->value |= uint64_t(s[i]) << (0x08 * i);
            }
            if (std::is_constant_evaluated()) {
               if (i == 8 && s[8]) // don't allow inputs longer than eight characters
                  throw;
            }
         }
         
         constexpr eight_cc& operator=(const eight_cc& other) noexcept {
            this->value = other.value;
            return *this;
         }
         
         constexpr operator uint64_t() const noexcept { return this->value; }
   };
}