#pragma once
#include <vector>
#include "codegen/rechunked/chunks/base.h"

namespace codegen {
   class decl_descriptor;
}

namespace codegen::rechunked::chunks {
   class qualified_decl : public base {
      public:
         static constexpr const type chunk_type = type::qualified_decl;
         virtual type get_type() const noexcept override { return chunk_type; }
         
         virtual bool compare(const base&) const noexcept override;
      
      public:
         std::vector<const decl_descriptor*> descriptors;
   };
}