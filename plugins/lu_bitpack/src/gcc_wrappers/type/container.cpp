#include "gcc_wrappers/type/container.h"
#include "gcc_wrappers/_node_boilerplate-impl.define.h"
#include <c-tree.h> // C_TYPE_BEING_DEFINED

namespace gcc_wrappers::type {
   GCC_NODE_WRAPPER_BOILERPLATE(container)
   
   chain container::member_chain() const {
      return chain(TYPE_FIELDS(this->_node));
   }
   
   bool container::is_still_being_defined() const {
      return C_TYPE_BEING_DEFINED(this->_node) != 0;
   }
}