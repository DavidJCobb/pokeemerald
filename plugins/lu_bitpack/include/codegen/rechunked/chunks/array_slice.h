#pragma once
#include "codegen/rechunked/chunks/base.h"
#include "codegen/array_access_info.h"

namespace codegen::rechunked::chunks {
   class array_slice : public base {
      public:
         static constexpr const type chunk_type = type::array_slice;
         virtual type get_type() const noexcept override { return chunk_type; }
         
         virtual bool compare(const base&) const noexcept override;
      
      public:
         array_access_info data;
   };
}