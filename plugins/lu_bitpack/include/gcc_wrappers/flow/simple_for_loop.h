#pragma once
#include "gcc_wrappers/decl/label.h"
#include "gcc_wrappers/decl/variable.h"
#include "gcc_wrappers/expr/local_block.h"
#include "gcc_wrappers/type/integral.h"

namespace gcc_wrappers {
   namespace flow {
      class simple_for_loop {
         public:
            simple_for_loop(type::integral counter_type);
         
            // Declarations are created when the loop itself is created, 
            // so you can reference the `break` and `continue` labels and 
            // the loop counter variable in your loop body.
            decl::variable counter;
            struct {
               intmax_t  start     = 0;
               uintmax_t last      = 0;
               intmax_t  increment = 1;
            } counter_bounds;
            //
            decl::label label_break;
            decl::label label_continue;
            decl::label label_condition;
            
            // This is the result; after you `bake` the for loop, you can append 
            // this to the statement loop for your function.
            expr::local_block enclosing;
            
            void bake(statement_list&& loop_body);
      };
   }
}