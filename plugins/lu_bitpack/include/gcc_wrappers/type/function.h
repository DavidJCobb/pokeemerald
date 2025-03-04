#pragma once
#include <array>
#include "gcc_wrappers/type/base.h"
#include "gcc_wrappers/list_node.h"
#include "gcc_wrappers/_node_boilerplate.define.h"

namespace gcc_wrappers::type {
   class function : public base {
      public:
         static bool raw_node_is(tree t) {
            return TREE_CODE(t) == FUNCTION_TYPE;
         }
         GCC_NODE_WRAPPER_BOILERPLATE(function)
         
      public:
         template<typename... Args> requires (std::is_base_of_v<base, Args> && ...)
         function(base return_type, Args... args) {
            this->_node = build_function_type_list( // tree.h
               return_type.unwrap(),
               args.unwrap()...,
               NULL_TREE
            );
         };
         
         base return_type() const;
         
         // Returns a list_node of raw arguments; the values are the argument types.
         // If this is not a varargs function, then the last element is the void type.
         //
         // The pair keys are the arguments' default values (in C++, if provided), 
         // and the pair values are the argument types. A default value may be an 
         // `error_mark_node`, in the event of syntax errors and similar.
         optional_list_node arguments() const;
         
         // We index arguments from zero. GCC likes to index them from one, at 
         // least in some places.
         base nth_argument_type(size_t n) const;
         
         optional_node nth_argument_default_value(size_t n) const;
         
         // Made legal in C++23: void foo(...);
         bool is_varargs_only() const;
         
         bool is_varargs() const;
         
         // Does not include varargs; does include `this` for member functions.
         size_t fixed_argument_count() const;
         
         // weird C-only jank: `void f()` takes a variable number of arguments
         // (but isn't varargs??)
         bool is_unprototyped() const;
         
         // Supported functor signatures:
         //  - void functor(gcc_wrappers::type::base);
         //  - void functor(gcc_wrappers::type::base, size_t index);
         // You can have a return type if you like, but we'll ignore it.
         template<typename Functor>
         void for_each_argument_type(Functor&& functor) const;
         
         template<size_t ArgCount>
         bool has_signature(
            bool          allow_unprototyped,
            bool          allow_varargs,
            bool          consider_implicit_argument_conversions,
            optional_base desired_return_type,
            std::array<base, ArgCount> argument_types
         );
   };
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(function);
}

#include "gcc_wrappers/_node_boilerplate.undef.h"
#include "gcc_wrappers/type/function.inl"