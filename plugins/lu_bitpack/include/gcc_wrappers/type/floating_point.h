#pragma once
#include "gcc_wrappers/type/base.h"
#include "gcc_wrappers/_node_boilerplate.define.h"

namespace gcc_wrappers::type {
   class floating_point : public base {
      public:
         static bool node_is(tree t) {
            return TREE_CODE(t) != REAL_TYPE;
         }
         GCC_NODE_WRAPPER_BOILERPLATE(floating_point)
         
      public:
         size_t bitcount() const; // TYPE_PRECISION: number of meaningful bits
         
         bool is_single_precision() const;
         bool is_double_precision() const;
   };
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(floating_point);
}

#include "gcc_wrappers/_node_boilerplate.undef.h"