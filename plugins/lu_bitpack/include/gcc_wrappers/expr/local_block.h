#pragma once
#include "gcc_wrappers/expr/base.h"
#include "gcc_wrappers/type/base.h"
#include "gcc_wrappers/statement_list.h"
#include "gcc_wrappers/_node_boilerplate.define.h"

namespace gcc_wrappers::expr {
   //
   // NOTE: Our current design for generating GENERIC from scratch is:
   //
   //  - We defer creation of a BIND_EXPR's BLOCK node until the BIND_EXPR 
   //    (or an ancestor BIND_EXPR) is made the root block of a FUNCTION_DECL.
   //
   //  - We likewise defer setting BIND_EXPR_VARS until a block is assigned.
   //
   class local_block : public base {
      public:
         static bool raw_node_is(tree t) {
            return TREE_CODE(t) == BIND_EXPR;
         }
         GCC_NODE_WRAPPER_BOILERPLATE(local_block)
      
      public:
         local_block(); // void-type with allocated statement list
         local_block(type::base); // with allocated statement list
         local_block(statement_list&&);
         local_block(type::base, statement_list&&);
         
         statement_list statements();
   };
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(local_block);
}

#include "gcc_wrappers/_node_boilerplate.undef.h"