#include "gcc_wrappers/type/helpers/lookup_by_name.h"
#include "gcc_wrappers/identifier.h"
#include <c-family/c-common.h> // identifier_global_tag, lookup_name

namespace gcc_wrappers::type {
   extern optional_base lookup_by_name(lu::strings::zview name) {
      return lookup_by_name(identifier(name));
   }
   extern optional_base lookup_by_name(identifier id_node) {
      auto node    = lookup_name(id_node.unwrap());
      //
      // The `lookup_name` function returns a `*_DECL` node with the given 
      // identifier. However, TYPE_DECLs only exist in C when the `typedef` 
      // keyword is used. To find normal struct definitions, we need to use 
      // another approach.
      //
      if (node == NULL_TREE) {
         //
         // That "other approach" is to look for a "tag."
         //
         node = identifier_global_tag(id_node.unwrap());
         if (node == NULL_TREE)
            return {};
      }
      if (TREE_CODE(node) == TYPE_DECL) {
         node = TREE_TYPE(node);
      } else if (!TYPE_P(node)) {
         return {};
      }
      return node;
   }
}