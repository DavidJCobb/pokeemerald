#include "gcc_wrappers/expr/ternary.h"
#include "gcc_wrappers/_node_boilerplate-impl.define.h"

namespace gcc_wrappers::expr {
   GCC_NODE_WRAPPER_BOILERPLATE(ternary)
   
   ternary::ternary(
      type::base    result_type,
      value         condition,
      optional_base if_true,
      optional_base if_false
   ) {
      this->_node = build3(
         COND_EXPR,
         result_type.unwrap(),
         condition.unwrap(),
         if_true.unwrap(),
         if_false.unwrap()
      );
   }
   
   optional_base ternary::get_condition() {
      return COND_EXPR_COND(this->_node);
   }
   optional_base ternary::get_true_branch() {
      return COND_EXPR_THEN(this->_node);
   }
   optional_base ternary::get_false_branch() {
      return COND_EXPR_ELSE(this->_node);
   }
   
   void ternary::set_true_branch(expr::base e) {
      COND_EXPR_THEN(this->_node) = e.unwrap();
   }
   void ternary::set_false_branch(expr::base e) {
      COND_EXPR_ELSE(this->_node) = e.unwrap();
   }
}