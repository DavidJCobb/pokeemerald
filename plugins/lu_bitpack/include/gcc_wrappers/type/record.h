#pragma once
#include "gcc_wrappers/type/container.h"
#include "gcc_wrappers/_node_boilerplate.define.h"

namespace gcc_wrappers::type {
   class record : public container {
      public:
         static bool raw_node_is(tree t) {
            return TREE_CODE(t) == RECORD_TYPE;
         }
         GCC_NODE_WRAPPER_BOILERPLATE(record)
         
      public:
   };
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(record);
}

#include "gcc_wrappers/_node_boilerplate.undef.h"