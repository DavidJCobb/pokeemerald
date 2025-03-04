#pragma once
#include "gcc_wrappers/expr/base.h"
#include "gcc_wrappers/expr/ternary.h"
#include "gcc_wrappers/type/base.h"

namespace gcc_wrappers {
   namespace flow {
      class simple_if_else_set {
         protected:
            expr::optional_ternary previous;
            type::base             type;
            bool did_else = false;
         
         public:
            simple_if_else_set(type::optional_base result_type = {});
            
            // This is what you'd want to append to a local block somewhere.
            expr::optional_ternary result;
            
            void add_branch(value condition, expr::base branch);
            
            // Adds a final "else" branch. Asserts that at least one branch 
            // already exists.
            void set_else_branch(expr::base branch);
      };
   }
}