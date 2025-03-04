#include "gcc_wrappers/decl/type_def.h"
#include "gcc_wrappers/_node_boilerplate-impl.define.h"

namespace gcc_wrappers::decl {
   GCC_NODE_WRAPPER_BOILERPLATE(type_def)
   
   type::optional_base type_def::declared() const {
      return TREE_TYPE(this->_node);
   }
   type::optional_base type_def::is_synonym_of() const {
      return DECL_ORIGINAL_TYPE(this->_node);
   }
}