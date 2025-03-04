#include "gcc_wrappers/flow/simple_if_else_set.h"
#include <gcc-plugin.h>
#include <tree.h> // *_type_node
#include <c-family/c-common.h> // *_type_node

namespace gcc_wrappers::flow {
   simple_if_else_set::simple_if_else_set(type::optional_base t)
   :
      type(t ? *t : type::base::wrap(void_type_node))
   {}
   
   void simple_if_else_set::add_branch(value condition, expr::base branch) {
      assert(!this->did_else);
      auto cond = expr::ternary(
         this->type,
         condition,
         branch
      );
      if (this->previous) {
         this->previous->set_false_branch(cond);
      } else {
         this->result = cond;
      }
      this->previous = cond;
   }

   void simple_if_else_set::set_else_branch(expr::base branch) {
      assert(!this->did_else);
      assert(this->previous);
      this->previous->set_false_branch(branch);
      this->did_else = true;
   }
}