#include "gcc_wrappers/expr/call.h"
#include "gcc_wrappers/_node_boilerplate-impl.define.h"

namespace gcc_wrappers::expr {
   GCC_NODE_WRAPPER_BOILERPLATE(call)
   
   decl::optional_function call::callee() const {
      return get_callee_fndecl(this->_node); // tree.h
   }
   
   size_t call::argument_count() const {
      return call_expr_nargs(this->_node);
   }
   value call::nth_argument(size_t n) const {
      assert(n < argument_count());
      return value::wrap(CALL_EXPR_ARG(this->_node, n));
   }
}