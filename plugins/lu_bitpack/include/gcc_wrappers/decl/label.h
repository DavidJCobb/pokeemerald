#pragma once
#include "gcc_wrappers/decl/base.h"
#include "gcc_wrappers/expr/declare_label.h"
#include "gcc_wrappers/value.h"
#include "gcc_wrappers/_node_boilerplate.define.h"

namespace gcc_wrappers::decl {
   class label : public base {
      public:
         static bool raw_node_is(tree t) {
            return TREE_CODE(t) == LABEL_DECL;
         }
         GCC_NODE_WRAPPER_BOILERPLATE(label)
         
      public:
         label(location_t source_location = UNKNOWN_LOCATION);
         
         expr::declare_label make_label_expr();
         
         // You're allowed to take the address of a label when using 
         // GCC extensions.
         value address_of();
   };
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(label);
}

#include "gcc_wrappers/_node_boilerplate.undef.h"