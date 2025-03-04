#include "gcc_wrappers/expr/base.h"
#include <cassert>
#include <c-family/c-common.h> // c_fully_fold
#include <fold-const.h> // fold
#include "gcc_headers/plugin-version.h"
#include "gcc_wrappers/_node_boilerplate-impl.define.h"

namespace gcc_wrappers::expr {
   GCC_NODE_WRAPPER_BOILERPLATE(base)
   
   location_t base::source_location() const {
      return EXPR_LOCATION(this->_node);
   }
   void base::set_source_location(location_t loc, bool wrap_if_necessary) {
      if (!CAN_HAVE_LOCATION_P(this->_node)) {
         this->_node = maybe_wrap_with_location(this->_node, loc);
         return;
      }
      protected_set_expr_location(this->_node, loc);
   }
   
   bool base::suppresses_unused_warnings() {
      return TREE_USED(this->_node) != 0;
   }
   void base::suppress_unused_warnings() {
      TREE_USED(this->_node) = 1;
   }
   void base::set_suppresses_unused_warnings(bool v) {
      TREE_USED(this->_node) = v;
   }
   
   type::base base::get_result_type() const {
      return type::base::wrap(TREE_TYPE(this->_node));
   }
   
   optional_value base::folded(bool manifestly_constant_evaluated) const {
      if (manifestly_constant_evaluated) {
         #if GCCPLUGIN_VERSION_MAJOR >= 12
            return ::fold_init(this->_node);
         #else
            auto node = fold_build1_initializer_loc(UNKNOWN_LOCATION, NOP_EXPR, TREE_TYPE(this->_node), this->_node);
            //
            // It's safe to abandon `node`; GCC uses garbage collection, 
            // so this shouldn't leak (for long).
            //
            auto op = TREE_OPERAND(node, 0);
            TREE_OPERAND(node, 0) = NULL_TREE;
            return op;
         #endif
      }
      return ::fold(this->_node);
   }
   
   optional_value base::fold_to_c_constant(
      bool initializer,
      bool as_lvalue
   ) const {
      bool maybe_const = true;
      auto expr = ::c_fully_fold(this->_node, initializer, &maybe_const, as_lvalue);
      if (!maybe_const)
         return {};
      return expr;
   }
   
   bool base::is_associative_operator() const {
      return associative_tree_code(this->code());
   }
   bool base::is_commutative_operator() const {
      return commutative_tree_code(this->code());
   }
   size_t base::operator_operand_count() const {
      return TREE_CODE_LENGTH(this->code());
   }
   
   bool base::is_comparison_oeprator() const {
      return COMPARISON_CLASS_P(this->_node) != 0;
   }
}