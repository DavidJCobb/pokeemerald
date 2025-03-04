#pragma once
#include <type_traits>
#include "gcc_wrappers/type/function.h"

namespace gcc_wrappers::type {
   template<typename Functor>
   void function::for_each_argument_type(Functor&& functor) const {
      function_args_iterator it;
      tree   raw;
      size_t i = 0;
      FOREACH_FUNCTION_ARGS(this->_node, raw, it) {
         if (raw == void_type_node || raw == error_mark_node)
            break;
         auto w = base::wrap(raw);
         if constexpr (std::is_invocable_v<Functor, base, size_t>) {
            functor(w, i++);
         } else {
            functor(w);
         }
      }
   }
   
   template<size_t ArgCount>
   bool function::has_signature(
      bool          allow_unprototyped,
      bool          allow_varargs,
      bool          consider_implicit_argument_conversions,
      optional_base desired_return_type,
      std::array<base, ArgCount> argument_types
   ) {
      if (this->is_unprototyped())
         return allow_unprototyped;
      
      if (desired_return_type) {
         if (this->return_type() != *desired_return_type)
            return false;
      }
      
      if constexpr (ArgCount == 0) {
         auto bare = TYPE_ARG_TYPES(this->_node);
         assert(bare != NULL_TREE); // should've been caught by `is_unprototyped`
         auto type = TREE_VALUE(bare);
         if (type == void_type_node) {
            return true;
         }
         return allow_varargs && this->is_varargs_only();
      } else {
         auto   args = *this->arguments();
         auto   it   = args.begin();
         size_t i    = 0;
         for(auto it = args.begin(); it != args.end(); ++i, ++it) {
            auto pair  = *it;
            auto maybe = pair.second;
            if (maybe.unwrap() == error_mark_node || !maybe) {
               return false;
            }
            auto type = (*maybe).as<base>();
            
            if (i >= ArgCount) {
               if (type.unwrap() == void_type_node) {
                  //
                  // Sentinel for the end of a non-varargs argument list.
                  //
                  return true;
               }
               //
               // Too many arguments seen.
               //
               return false;
            }
            if (type.unwrap() == void_type_node) {
               //
               // Too few arguments seen.
               //
               return false;
            }
            
            auto desired = argument_types[i];
            if (consider_implicit_argument_conversions) {
               bool assignable = desired.is_assignable_to(
                  type,
                  assign_check_options{
                     .as_function_argument = true,
                  }
               );
               if (!assignable)
                  return false;
            } else {
               if (type != desired)
                  return false;
            }
         }
         if (i + 1 < ArgCount) { // +1 to account for the `void` sentinel
            //
            // Too few arguments seen.
            //
            return false;
         }
         return allow_varargs;
      }
      return true;
   }
}