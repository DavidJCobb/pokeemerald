#pragma once
#include "codegen/instructions/base.h"

namespace codegen::instructions {
   class padding : public base {
      public:
         static constexpr const type node_type = type::padding;
         virtual type get_type() const noexcept override { return node_type; };
         
         virtual expr_pair generate(const utils::generation_context&) const;
      
      public:
         size_t bitcount = 0;
   };
}