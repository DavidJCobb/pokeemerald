#pragma once
#include "gcc_wrappers/expr/base.h"
#include "gcc_wrappers/type/base.h"
#include "gcc_wrappers/_node_boilerplate.define.h"

namespace gcc_wrappers::expr {
   class ternary : public base {
      public:
         static bool raw_node_is(tree t) {
            return TREE_CODE(t) == COND_EXPR;
         }
         GCC_NODE_WRAPPER_BOILERPLATE(ternary)
      
      public:
         ternary(
            type::base    result_type,
            value         condition,
            optional_base if_true = nullptr,
            optional_base if_false = nullptr
         );
         
         optional_base get_condition();
         optional_base get_true_branch();
         optional_base get_false_branch();
         
         void set_true_branch(expr::base);
         void set_false_branch(expr::base);
   };
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(ternary);
}

#include "gcc_wrappers/_node_boilerplate.undef.h"