#include "gcc_wrappers/expr/assign.h"
#include "gcc_wrappers/_node_boilerplate-impl.define.h"
#include <c-family/c-common.h>

namespace gcc_wrappers::expr {
   GCC_NODE_WRAPPER_BOILERPLATE(assign)
   
   assign::assign(const value& dst, const value& src) {
      auto dst_type = dst.value_type();
      
      // Do some of GCC's own validation ahead of time
      assert(dst_type.is_complete());
      assert(!dst_type.is_array());
      assert(dst.is_lvalue());
      
      // build_modify_expr declared in c-common.h, defined in c/c-typeck.cc
      this->_node = build_modify_expr(
         UNKNOWN_LOCATION,   // expression source location
         (tree)dst.unwrap(), // LHS
         NULL_TREE,          // original type of LHS (needed if it's an enum bitfield)
         NOP_EXPR,           // operation (e.g. PLUS_EXPR if we want to make a +=)
         UNKNOWN_LOCATION,   // RHS source location
         (tree)src.unwrap(), // RHS
         NULL_TREE           // original type of RHS (needed if it's an enum)
      );
      // casts needed due to `const_tree` versus `tree`
   }
   
   /*static*/ assign assign::make_with_modification(
      const value& dst,
      tree_code    operation,
      const value& src
   ) {
      return assign::wrap(
         build_modify_expr(
            UNKNOWN_LOCATION,   // expression source location
            (tree)dst.unwrap(), // LHS
            NULL_TREE,          // original type of LHS (needed if it's an enum bitfield)
            operation,          // operation (e.g. PLUS_EXPR if we want to make a +=)
            UNKNOWN_LOCATION,   // RHS source location
            (tree)src.unwrap(), // RHS
            NULL_TREE           // original type of RHS (needed if it's an enum)
         )
      );
      // casts needed due to `const_tree` versus `tree`
   }
   
   value assign::src() const {
      return value::wrap(TREE_OPERAND(this->_node, 0));
   }
   value assign::dst() const {
      return value::wrap(TREE_OPERAND(this->_node, 1));
   }
}