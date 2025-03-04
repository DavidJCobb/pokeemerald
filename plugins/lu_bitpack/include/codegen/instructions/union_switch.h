#pragma once
#include <cstdint>
#include <memory>
#include <unordered_map>
#include "codegen/instructions/base.h"
#include "codegen/instructions/union_case.h"
#include "codegen/value_path.h"

namespace codegen::instructions::utils {
   struct generation_context;
}

namespace codegen::instructions {
   class union_switch : public base {
      public:
         static constexpr const type node_type = type::union_switch;
         virtual type get_type() const noexcept override { return node_type; };
         
         virtual expr_pair generate(const utils::generation_context&) const;
      
      public:
         value_path condition_operand;
         std::unordered_map<intmax_t, std::unique_ptr<union_case>> cases;
         std::unique_ptr<union_case> else_case;
   };
}