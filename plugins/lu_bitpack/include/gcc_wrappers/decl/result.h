#pragma once
#include "gcc_wrappers/decl/base_value.h"
#include "gcc_wrappers/type/base.h"
#include "gcc_wrappers/_node_boilerplate.define.h"

namespace gcc_wrappers::decl {
   class result : public base_value {
      public:
         static bool raw_node_is(tree t) {
            return TREE_CODE(t) == RESULT_DECL;
         }
         GCC_NODE_WRAPPER_BOILERPLATE(result)
         
      public:
         result(type::base);
   };
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(result);
}

#include "gcc_wrappers/_node_boilerplate.undef.h"