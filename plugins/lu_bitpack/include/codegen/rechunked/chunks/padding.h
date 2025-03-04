#pragma once
#include "codegen/rechunked/chunks/base.h"

namespace codegen::rechunked::chunks {
   class padding : public base {
      public:
         static constexpr const type chunk_type = type::padding;
         virtual type get_type() const noexcept override { return chunk_type; }
         
         virtual bool compare(const base&) const noexcept override;
         
      public:
         size_t bitcount = 0;
   };
}