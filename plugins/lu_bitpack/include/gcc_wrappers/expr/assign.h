#pragma once
#include "gcc_wrappers/expr/base.h"
#include "gcc_wrappers/_node_boilerplate.define.h"

namespace gcc_wrappers::expr {
   class assign : public base {
      public:
         static bool raw_node_is(tree t) {
            return TREE_CODE(t) == MODIFY_EXPR;
         }
         GCC_NODE_WRAPPER_BOILERPLATE(assign)
      
      public:
         assign(const value& dst, const value& src);
         
         // Creates nested expressions for e.g. +=
         static assign make_with_modification(
            const value& dst,
            tree_code    operation,
            const value& src
         );
         
         value src() const;
         value dst() const;
   };
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(assign);
}

#include "gcc_wrappers/_node_boilerplate.undef.h"