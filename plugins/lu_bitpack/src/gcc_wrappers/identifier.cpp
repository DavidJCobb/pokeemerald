#include "gcc_wrappers/identifier.h"
#include <stringpool.h> // get_identifier
#include "gcc_wrappers/_node_boilerplate-impl.define.h"

namespace gcc_wrappers {
   GCC_NODE_WRAPPER_BOILERPLATE(identifier)
   
   /*static*/ bool identifier::raw_node_is(tree t) {
      return TREE_CODE(t) == IDENTIFIER_NODE;
   }
   
   identifier::identifier(lu::strings::zview name) {
      this->_node = get_identifier(name.c_str());
   }
   
   lu::strings::zview identifier::name() const {
      return lu::strings::zview(IDENTIFIER_POINTER(this->_node));
   }
}