#include "gcc_wrappers/type/function.h"
#include "gcc_headers/plugin-version.h"
#include "gcc_wrappers/_node_boilerplate-impl.define.h"

namespace gcc_wrappers::type {
   GCC_NODE_WRAPPER_BOILERPLATE(function)
   
   base function::return_type() const {
      return base::wrap(TREE_TYPE(this->_node));
   }
   
   optional_list_node function::arguments() const {
      return TYPE_ARG_TYPES(this->_node);
   }
   
   base function::nth_argument_type(size_t n) const {
      function_args_iterator it;
      function_args_iter_init(&it, this->_node);
      for(size_t i = 0; i < n; ++i) {
         function_args_iter_next(&it);
      }
      return base::wrap(function_args_iter_cond(&it));
   }
   
   optional_node function::nth_argument_default_value(size_t n) const {
      function_args_iterator it;
      function_args_iter_init(&it, this->_node);
      for(size_t i = 0; i < n; ++i) {
         function_args_iter_next(&it);
      }
      assert(it.next != NULL_TREE);
      auto arg_val = TREE_PURPOSE(it.next);
      if (arg_val == error_mark_node) {
         //
         // Set as the default value in case of syntax errors, e.g. user code 
         // like `void foo(int a, int b = 3, int c)`. The default value of `c` 
         // would be set to the error mark node.
         //
         arg_val = NULL_TREE;
      }
      return arg_val;
   }
   
   bool function::is_varargs_only() const {
      #if GCCPLUGIN_VERSION_MAJOR >= 13
         //
         // Check for functions that only take varargs. This was made legal 
         // in C23 (N2975) and supported from GCC 13 onward.
         //
         if (TYPE_NO_NAMED_ARGS_STDARG_P(this->_node))
            return true;
      #endif
      return false;
   }
   
   bool function::is_varargs() const {
      if (this->is_varargs_only())
         return true;
      
      auto args = this->arguments();
      if (!args)
         return false; // unprototyped
      
      auto back = args->back();
      if (TREE_VALUE(back.unwrap()) == void_list_node)
         return true;
      
      return false;
   }
   
   size_t function::fixed_argument_count() const {
      return type_num_arguments(this->_node);
   }
   
   bool function::is_unprototyped() const {
      return TYPE_ARG_TYPES(this->_node) == NULL_TREE;
   }
}