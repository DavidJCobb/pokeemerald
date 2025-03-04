#include "gcc_wrappers/type/floating_point.h"
#include "gcc_wrappers/_node_boilerplate-impl.define.h"

namespace gcc_wrappers::type {
   GCC_NODE_WRAPPER_BOILERPLATE(floating_point)
   
   size_t floating_point::bitcount() const {
      return TYPE_PRECISION(this->_node);
   }
   
   bool floating_point::is_single_precision() const {
      return bitcount() == 32;
   }
   bool floating_point::is_double_precision() const {
      return bitcount() == 64;
   }
}