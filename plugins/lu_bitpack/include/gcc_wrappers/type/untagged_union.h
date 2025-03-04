#pragma once
#include "gcc_wrappers/type/container.h"
#include "gcc_wrappers/_node_boilerplate.define.h"

namespace gcc_wrappers::type {
   class untagged_union : public container {
      public:
         static bool raw_node_is(tree t) {
            return TREE_CODE(t) == UNION_TYPE;
         }
         GCC_NODE_WRAPPER_BOILERPLATE(untagged_union)
         
      public:
   };
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(untagged_union);
}

#include "gcc_wrappers/_node_boilerplate.undef.h"