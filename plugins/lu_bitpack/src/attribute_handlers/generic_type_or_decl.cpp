#include "attribute_handlers/generic_type_or_decl.h"
#include <diagnostic.h>

namespace attribute_handlers {
   extern tree generic_type_or_decl(tree* node, tree name, tree args, int flags, bool* no_add_attrs) {
      if (!node || *node == NULL_TREE) {
         return NULL_TREE;
      }
      if (TYPE_P(*node)) {
         if (flags & ATTR_FLAG_FUNCTION_NEXT) {
            goto not_allowed_here;
         }
         if (flags & ATTR_FLAG_DECL_NEXT) {
            *no_add_attrs = true;
            return tree_cons(name, args, NULL_TREE);
         }
         *no_add_attrs = false;
         return NULL_TREE;
      }
      switch (TREE_CODE(*node)) {
         case TYPE_DECL:
         case FIELD_DECL:
         case VAR_DECL:
            *no_add_attrs = false;
            return NULL_TREE;
         default:
            break;
      }
      
      //
      // Failure cases:
      //
   not_allowed_here:
      *no_add_attrs = true;
      warning(OPT_Wattributes, "%qE attribute only applies to types and struct fields", name);
      return NULL_TREE;
   }
}