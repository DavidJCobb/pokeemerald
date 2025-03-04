#pragma once
#include <type_traits>
#include "gcc_wrappers/expr/base.h"
#include "gcc_wrappers/decl/function.h"
#include "gcc_wrappers/type/function.h"
#include "gcc_wrappers/_node_boilerplate.define.h"

namespace gcc_wrappers::expr {
   class call : public base {
      public:
         static bool raw_node_is(tree t) {
            return TREE_CODE(t) == CALL_EXPR;
         }
         GCC_NODE_WRAPPER_BOILERPLATE(call)
      
      public:
         template<typename... Args> requires (std::is_base_of_v<value, Args> && ...)
         call(decl::function func_decl, Args... args) {
            auto func_type = func_decl.function_type();
            if (!func_type.is_unprototyped()) {
               //
               // Verify argcount. Note that C does not support default argument 
               // initializers, so a function that takes N args must receive N 
               // explicitly-specified args.
               //
               if (func_type.is_varargs()) {
                  assert(sizeof...(Args) >= func_type.fixed_argument_count());
               } else {
                  assert(sizeof...(Args) == func_type.fixed_argument_count());
               }
            }
            this->_node = build_call_expr(
               func_decl.unwrap(),
               sizeof...(Args),
               args.unwrap()...
            );
         }
         
         // estimate; may fail
         decl::optional_function callee() const; // get_callee_fndecl
         
         size_t argument_count() const;
         value nth_argument(size_t) const; // asserts in-bounds
   };
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(call);
}

#include "gcc_wrappers/_node_boilerplate.undef.h"