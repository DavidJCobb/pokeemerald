#pragma once
#include "codegen/instructions/base.h"
#include "codegen/expr_pair.h"
#include "codegen/value_path.h"

namespace codegen::instructions {
   class single : public base {
      public:
         static constexpr const type node_type = type::single;
         virtual type get_type() const noexcept override { return node_type; };
         
         virtual expr_pair generate(const utils::generation_context&) const;
      
      public:
         value_path value;
         
         bool is_omitted_and_defaulted() const;
   };
}