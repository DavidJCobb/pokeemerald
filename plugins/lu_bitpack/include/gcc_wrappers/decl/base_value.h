#pragma once
#include <string_view>
#include "gcc_wrappers/decl/base.h"
#include "gcc_wrappers/type/base.h"
#include "gcc_wrappers/_node_boilerplate.define.h"

namespace gcc_wrappers {
   namespace type {
      class base;
   }
   class scope;
   class value;
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(value);
}

namespace gcc_wrappers::decl {
   //
   // DECL types that can be used as values. Note that you must 
   // explicitly cast them via `as_value`.
   //
   class base_value : public base {
      public:
         static bool raw_node_is(tree t);
         GCC_NODE_WRAPPER_BOILERPLATE(base_value)
         
      public:
         bool linkage_status_unknown() const;
         
         type::base value_type() const;
         
         value as_value();
            
         bool is_read_only() const;
         void make_read_only();
         void set_is_read_only(bool);
   };
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(base_value);
}

#include "gcc_wrappers/_node_boilerplate.undef.h"