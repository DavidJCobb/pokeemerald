#include "gcc_wrappers/type/enumeration.h"
#include "gcc_wrappers/_node_boilerplate-impl.define.h"
#include "gcc_headers/plugin-version.h"
#include <c-tree.h> // C_TYPE_BEING_DEFINED

namespace gcc_wrappers::type {
   GCC_NODE_WRAPPER_BOILERPLATE(enumeration)
   
   [[nodiscard]] std::vector<enumeration::member> enumeration::all_enum_members() const {
      std::vector<member> list;
      for_each_enum_member([&list](const member& item) {
         list.push_back(item);
      });
      return list;
   }
         
   bool enumeration::is_opaque() const {
      return ENUM_IS_OPAQUE(this->_node);
   }
   bool enumeration::is_scoped() const {
      return ENUM_IS_SCOPED(this->_node);
   }
   
   base enumeration::underlying_type() const {
      #if GCCPLUGIN_VERSION_MAJOR >= 13 && GCCPLUGIN_VERSION_MINOR >= 1
         return base::wrap(ENUM_UNDERLYING_TYPE(this->_node));
      #else
         return base::wrap(integer_type_node); // should be in <tree.h>
      #endif
   }
   bool enumeration::has_explicit_underlying_type() const {
      #if GCCPLUGIN_VERSION_MAJOR >= 13 && GCCPLUGIN_VERSION_MINOR >= 1
         return ENUM_FIXED_UNDERLYING_TYPE_P(this->_node);
      #else
         return false;
      #endif
   }
   
   bool enumeration::is_still_being_defined() const {
      return C_TYPE_BEING_DEFINED(this->_node) != 0;
   }
}