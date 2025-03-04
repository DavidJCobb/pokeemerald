#include "gcc_wrappers/flow/simple_for_loop.h"
#include "gcc_wrappers/constant/integer.h"
#include "gcc_wrappers/expr/assign.h"
#include "gcc_wrappers/expr/declare_label.h"
#include "gcc_wrappers/expr/go_to_label.h"
#include "gcc_wrappers/expr/ternary.h"
#include "gcc_wrappers/environment/c_family/language.h"
#include <c-family/c-common.h> // LABEL_DECL_BREAK, LABEL_DECL_CONTINUE

namespace gcc_wrappers::flow {
   simple_for_loop::simple_for_loop(type::integral counter_type)
      :
      counter("__i", counter_type)
   {
      this->counter.make_artificial();
      switch (environment::c_family::current_language()) {
         case environment::c_family::language::c:
         case environment::c_family::language::objective_c:
            LABEL_DECL_BREAK(this->label_break.unwrap()) = 1;
            LABEL_DECL_CONTINUE(this->label_continue.unwrap()) = 1;
            break;
         default:
            break;
      }
   }
   
   void simple_for_loop::bake(statement_list&& loop_body) {
      auto counter_type = this->counter.value_type().as_integral();
      auto statements   = this->enclosing.statements();
      
      /*
      
         Produces:
         
         {
            int __i = start;
            goto l_condition;
         l_body:
            // ...
         l_continue:
            __i += increment;
         l_condition:
            (__i <= last) ? goto l_body : goto l_break;
         l_break:
         }
         
      */
      
      this->counter.make_used();
      statements.append(this->counter.make_declare_expr());
      this->counter.set_initial_value(constant::integer(counter_type, this->counter_bounds.start));
      
      decl::label l_body;
      
      statements.append(expr::go_to_label(this->label_condition));
      statements.append(expr::declare_label(l_body));
      statements.append(std::move(loop_body));
      statements.append(expr::declare_label(this->label_continue));
      statements.append(
         this->counter.as_value().increment_pre(
            constant::integer(counter_type, this->counter_bounds.increment)
         ).as_expr()
      );
      statements.append(expr::declare_label(this->label_condition));
      statements.append(
         expr::ternary(
            type::base::wrap(void_type_node),
            this->counter.as_value().cmp_is_less_or_equal(
               constant::integer(counter_type, this->counter_bounds.last)
            ),
            expr::go_to_label(l_body),
            expr::go_to_label(this->label_break)
         )
      );
      statements.append(expr::declare_label(this->label_break));
   }
}