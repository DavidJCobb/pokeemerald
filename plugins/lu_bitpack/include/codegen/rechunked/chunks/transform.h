#pragma once
#include <vector>
#include "codegen/rechunked/chunks/base.h"
#include "gcc_wrappers/type/base.h"

namespace codegen::rechunked::chunks {
   class transform : public base {
      public:
         static constexpr const type chunk_type = type::transform;
         virtual type get_type() const noexcept override { return chunk_type; }
         
         virtual bool compare(const base&) const noexcept override;
      
      public:
         std::vector<gcc_wrappers::type::base> types;
   };
}