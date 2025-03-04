#pragma once
#include "gcc_wrappers/decl/base_value.h"
#include "gcc_wrappers/type/base.h"
#include "gcc_wrappers/value.h"
#include "gcc_wrappers/_node_boilerplate.define.h"

namespace gcc_wrappers::decl {
   class param : public base_value {
      public:
         static bool raw_node_is(tree t) {
            return TREE_CODE(t) == PARM_DECL;
         }
         GCC_NODE_WRAPPER_BOILERPLATE(param)
         
      public:
         type::base effective_type() const;
         
         bool is_ever_read() const;
         void make_ever_read();
         void set_is_ever_read(bool);
   };
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(param);
}

#include "gcc_wrappers/_node_boilerplate.undef.h"