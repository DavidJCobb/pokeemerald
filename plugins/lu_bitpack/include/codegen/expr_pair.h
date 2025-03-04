#pragma once
#include "gcc_wrappers/expr/base.h"

namespace codegen {
   struct optional_expr_pair {
      gcc_wrappers::expr::optional_base read;
      gcc_wrappers::expr::optional_base save;
   };
   
   struct expr_pair {
      expr_pair(const expr_pair&) = default;
      expr_pair(expr_pair&&) noexcept = default;
      
      expr_pair(const optional_expr_pair& pair);
      expr_pair(gcc_wrappers::expr::base read, gcc_wrappers::expr::base save);
      
      gcc_wrappers::expr::base read;
      gcc_wrappers::expr::base save;
   };
}