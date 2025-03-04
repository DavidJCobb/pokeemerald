#include "gcc_wrappers/decl/param.h"
#include "gcc_wrappers/_node_boilerplate-impl.define.h"

namespace gcc_wrappers::decl {
   GCC_NODE_WRAPPER_BOILERPLATE(param)
   
   type::base param::effective_type() const {
      return type::base::wrap(DECL_ARG_TYPE(this->_node));
   }
   
   bool param::is_ever_read() const {
      return DECL_READ_P(this->_node);
   }
   void param::make_ever_read() {
      set_is_ever_read(true);
   }
   void param::set_is_ever_read(bool v) {
      DECL_READ_P(this->_node) = v ? 1 : 0;
   }
}