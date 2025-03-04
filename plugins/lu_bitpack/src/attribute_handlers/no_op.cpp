#include "attribute_handlers/no_op.h"

namespace attribute_handlers {
   extern tree no_op(tree* node, tree name, tree args, int flags, bool* no_add_attrs) {
      *no_add_attrs = false;
      return NULL_TREE;
   }
}