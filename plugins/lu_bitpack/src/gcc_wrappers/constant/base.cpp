#include "gcc_wrappers/constant/base.h"
#include <cassert>
#include "gcc_wrappers/_node_boilerplate-impl.define.h"

namespace gcc_wrappers::constant {
   GCC_NODE_WRAPPER_BOILERPLATE(base)
   
   type::base base::value_type() const {
      return type::base::wrap(TREE_TYPE(this->_node));
   }
}