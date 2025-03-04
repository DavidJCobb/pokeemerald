#include "codegen/rechunked/chunks/array_slice.h"

namespace codegen::rechunked::chunks {
   /*virtual*/ bool array_slice::compare(const base& other) const noexcept /*override*/ {
      const auto* casted = other.as<array_slice>();
      if (!casted)
         return false;
      
      return this->data == casted->data;
   }
}