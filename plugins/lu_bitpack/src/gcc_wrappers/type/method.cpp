#include "gcc_wrappers/type/method.h"
#include "gcc_wrappers/_node_boilerplate-impl.define.h"

namespace gcc_wrappers::type {
   GCC_NODE_WRAPPER_BOILERPLATE(method)
   
   base method::is_method_of() const {
      return base::wrap(TYPE_METHOD_BASETYPE(this->_node));
   }
}