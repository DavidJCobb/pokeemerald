#include "codegen/rechunked/chunks/padding.h"

namespace codegen::rechunked::chunks {
   /*virtual*/ bool padding::compare(const base& other) const noexcept /*override*/ {
      const auto* casted = other.as<padding>();
      if (!casted)
         return false;
      
      return this->bitcount == casted->bitcount;
   }
}