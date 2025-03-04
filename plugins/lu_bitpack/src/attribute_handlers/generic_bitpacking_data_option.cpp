#include "attribute_handlers/generic_bitpacking_data_option.h"
#include "attribute_handlers/generic_type_or_decl.h"
#include "basic_global_state.h"

namespace attribute_handlers {
   extern tree generic_bitpacking_data_option(tree* node_ptr, tree name, tree args, int flags, bool* no_add_attrs) {
      *no_add_attrs = false;
      
      auto result = generic_type_or_decl(node_ptr, name, args, flags, no_add_attrs);
      if (*no_add_attrs) {
         return result;
      }
      
      auto& state = basic_global_state::get();
      state.on_attribute_seen(node_ptr[0]);
      if (!state.enabled) {
         *no_add_attrs = true;
      }
      
      return NULL_TREE;
   }
}