#pragma once
#include "gcc_wrappers/expr/base.h"
#include "gcc_wrappers/_node_boilerplate.define.h"

namespace gcc_wrappers::decl {
   class variable;
}

namespace gcc_wrappers::expr {
   class declare : public base {
      public:
         static bool raw_node_is(tree t) {
            return TREE_CODE(t) == DECL_EXPR;
         }
         GCC_NODE_WRAPPER_BOILERPLATE(declare)
      
      public:
         declare(decl::variable&, location_t source_location = UNKNOWN_LOCATION);
   };
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(declare);
}

#include "gcc_wrappers/_node_boilerplate.undef.h"