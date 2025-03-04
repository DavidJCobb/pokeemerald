#pragma once
#include "gcc_wrappers/decl/base.h"
#include "gcc_wrappers/type/base.h"
#include "gcc_wrappers/_node_boilerplate.define.h"

namespace gcc_wrappers::decl {
   class type_def : public base {
      public:
         static bool raw_node_is(tree t) {
            return TREE_CODE(t) == TYPE_DECL;
         }
         GCC_NODE_WRAPPER_BOILERPLATE(type_def)
         
      public:
         type::optional_base declared() const;      // `b` in `typedef a b`
         type::optional_base is_synonym_of() const; // `a` in `typedef a b`. return value may be empty.
   };
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(type_def);
}

#include "gcc_wrappers/_node_boilerplate.undef.h"