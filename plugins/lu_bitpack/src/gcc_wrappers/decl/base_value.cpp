#include "gcc_wrappers/decl/base_value.h"
#include "gcc_wrappers/value.h"
#include "gcc_wrappers/_node_boilerplate-impl.define.h"
#include <cassert>

namespace gcc_wrappers::decl {
   GCC_NODE_WRAPPER_BOILERPLATE(base_value)
   
   /*static*/ bool base_value::raw_node_is(tree t) {
      switch (TREE_CODE(t)) {
         case FIELD_DECL:
         case PARM_DECL:
         case RESULT_DECL:
         case VAR_DECL:
            return true;
         default:
            break;
      }
      return false;
   }
   
   bool base_value::linkage_status_unknown() const {
      return DECL_DEFER_OUTPUT(this->_node);
   }
   
   value base_value::as_value() {
      return value::wrap(this->_node);
   }
   
   type::base base_value::value_type() const {
      return type::base::wrap(TREE_TYPE(this->_node));
   }
   
   bool base_value::is_read_only() const {
      return TREE_READONLY(this->_node) != 0;
   }
   void base_value::make_read_only() {
      set_is_read_only(true);
   }
   void base_value::set_is_read_only(bool v) {
      TREE_READONLY(this->_node) = v;
   }
}