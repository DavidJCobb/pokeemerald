#include "codegen/rechunked/chunks/transform.h"

namespace codegen::rechunked::chunks {
   /*virtual*/ bool transform::compare(const base& other) const noexcept /*override*/ {
      const auto* casted = other.as<transform>();
      if (!casted)
         return false;
      
      return this->types == casted->types;
   }
}