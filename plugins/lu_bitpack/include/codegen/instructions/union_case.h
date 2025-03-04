#pragma once
#include "codegen/instructions/base.h"

namespace codegen::instructions {
   class union_case : public container {
      public:
         static constexpr const type node_type = type::union_case;
         virtual type get_type() const noexcept override { return node_type; };
   };
}