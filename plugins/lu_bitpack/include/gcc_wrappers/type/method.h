#pragma once
#include "gcc_wrappers/type/function.h"
#include "gcc_wrappers/_node_boilerplate.define.h"

namespace gcc_wrappers::type {
   class method : public function {
      public:
         static bool raw_node_is(tree t) {
            return TREE_CODE(t) == METHOD_TYPE;
         }
         GCC_NODE_WRAPPER_BOILERPLATE(method)
         
      public:
         base is_method_of() const;
   };
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(method);
}

#include "gcc_wrappers/_node_boilerplate.undef.h"