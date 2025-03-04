#pragma once
#include <type_traits>
#include "gcc_wrappers/type/base.h"
#include "gcc_wrappers/value.h"
#include "gcc_wrappers/_node_boilerplate.define.h"

namespace gcc_wrappers {
   namespace constant {
      class base : public value {
         GCC_NODE_WRAPPER_BOILERPLATE(base)
         public:
            static bool raw_node_is(tree t) {
               return CONSTANT_CLASS_P(t);
            }
         
         public:
            type::base value_type() const;
      };
      DECLARE_GCC_OPTIONAL_NODE_WRAPPER(base);
   }
}

#include "gcc_wrappers/_node_boilerplate.undef.h"