#include "gcc_wrappers/expr/return_result.h"
#include "gcc_wrappers/decl/result.h"
#include "gcc_wrappers/expr/assign.h"
#include "gcc_wrappers/_node_boilerplate-impl.define.h"

namespace gcc_wrappers::expr {
   GCC_NODE_WRAPPER_BOILERPLATE(return_result)
   
   return_result::return_result(decl::result res) {
      this->_node = build1(
         RETURN_EXPR,
         res.value_type().unwrap(),
         res.unwrap()
      );
   }
   return_result::return_result(expr::assign expr) {
      assert(decl::result::raw_node_is(expr.dst().unwrap()));
      this->_node = build1(
         RETURN_EXPR,
         expr.value_type().unwrap(),
         expr.unwrap()
      );
   }
}