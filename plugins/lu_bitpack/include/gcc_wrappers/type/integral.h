#pragma once
#include "gcc_wrappers/type/base.h"
#include "gcc_wrappers/_node_boilerplate.define.h"

namespace gcc_wrappers::constant {
   class integer;
}

namespace gcc_wrappers::type {
   class integral : public base {
      public:
         static bool raw_node_is(tree t) {
            switch (TREE_CODE(t)) {
               case BOOLEAN_TYPE:
               case ENUMERAL_TYPE:
               case INTEGER_TYPE:
                  return true;
               default:
                  break;
            }
            return false;
         }
         GCC_NODE_WRAPPER_BOILERPLATE(integral)
         
      public:
         size_t bitcount() const; // TYPE_PRECISION
         
         bool is_signed() const;
         bool is_unsigned() const;
         
         intmax_t minimum_value() const;
         uintmax_t maximum_value() const;
         
         integral make_signed() const;
         integral make_unsigned() const;
         
         bool can_hold_value(const constant::integer) const;
   };
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(integral);
}

#include "gcc_wrappers/_node_boilerplate.undef.h"