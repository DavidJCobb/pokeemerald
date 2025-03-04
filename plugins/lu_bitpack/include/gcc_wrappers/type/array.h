#pragma once
#include <optional>
#include "gcc_wrappers/type/base.h"
#include "gcc_wrappers/_node_boilerplate.define.h"

namespace gcc_wrappers::type {
   class array : public base {
      public:
         static bool raw_node_is(tree t) {
            return TREE_CODE(t) == ARRAY_TYPE;
         }
         GCC_NODE_WRAPPER_BOILERPLATE(array)
         
      public:
         bool is_variable_length_array() const;
         
         // returns empty if a variable-length array.
         std::optional<size_t> extent() const;
         
         // 1 for foo[n]; 2 for foo[m][n]; etc.
         size_t rank() const;
         
         base value_type() const;
   };
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(array);
}

#include "gcc_wrappers/_node_boilerplate.undef.h"