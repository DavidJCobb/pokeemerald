#include "codegen/rechunked/chunks/qualified_decl.h"

namespace codegen::rechunked::chunks {
   /*virtual*/ bool qualified_decl::compare(const base& other) const noexcept /*override*/ {
      const auto* casted = other.as<qualified_decl>();
      if (!casted)
         return false;
      
      return this->descriptors == casted->descriptors;
   }
}