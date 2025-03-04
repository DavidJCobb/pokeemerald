#pragma once
#include <type_traits>
#include <vector>
#include "gcc_wrappers/decl/base.h"
#include "gcc_wrappers/decl/field.h"
#include "gcc_wrappers/decl/param.h"
#include "gcc_wrappers/decl/type_def.h"
#include "gcc_wrappers/decl/variable.h"
#include "gcc_wrappers/type/base.h"
#include "gcc_wrappers/type/array.h"

namespace bitpacking {
   template<typename Functor> requires std::is_invocable_v<Functor, gcc_wrappers::node>
   bool for_each_influencing_entity(gcc_wrappers::type::base type, Functor&& functor) {
      constexpr const bool can_abort = std::is_invocable_r_v<Functor, bool, gcc_wrappers::node>;
      
      if (type.is_array()) {
         if (!for_each_influencing_entity(type.as_array().value_type(), functor))
            return false;
      }
      auto decl = type.declaration();
      if (!decl) {
         //
         // Shortcut: if this type doesn't have an associated typedef, then we 
         // can skip all the code to handle transitive typedefs in order from 
         // "oldest" to "newest," and just run the functor right on the type.
         //
         if constexpr (can_abort) {
            return functor(type);
         } else {
            functor(type);
         }
         return true;
      }
      //
      // Handle transitive typedefs. For example, given `typedef a b`, we want 
      // to apply bitpacking options for `a` when we see `b`, before we apply 
      // any bitpacking options for `b`.
      //
      std::vector<gcc_wrappers::type::base> transitives;
      transitives.push_back(type);
      do {
         auto tran = decl->is_synonym_of();
         if (!tran)
            break;
         transitives.push_back(*tran);
         decl = tran->declaration();
      } while (decl);
      for(auto it = transitives.rbegin(); it != transitives.rend(); ++it) {
         auto tran = *it;
         auto decl = tran.declaration();
         if constexpr (can_abort) {
            if (!functor(tran))
               return false;
         } else {
            functor(tran);
         }
         if (decl) {
            if constexpr (can_abort) {
               if (!functor(*decl))
                  return false;
            } else {
               functor(*decl);
            }
         }
      }
      return true;
   }
   
   template<typename Functor, typename NodeWrapper> requires (
      std::is_invocable_v<Functor, gcc_wrappers::node> &&
      std::is_same_v<NodeWrapper, gcc_wrappers::decl::field> ||
      std::is_same_v<NodeWrapper, gcc_wrappers::decl::param> ||
      std::is_same_v<NodeWrapper, gcc_wrappers::decl::variable>
   )
   void for_each_influencing_entity(NodeWrapper decl, Functor&& functor) {
      for_each_influencing_entity(decl.value_type(), functor);
      if constexpr (std::is_invocable_r_v<Functor, bool, gcc_wrappers::node>) {
         if (!functor(decl))
            return;
      } else {
         functor(decl);
      }
   }
   
   template<typename Functor> requires std::is_invocable_v<Functor, gcc_wrappers::node>
   void for_each_influencing_entity(gcc_wrappers::decl::type_def decl, Functor&& functor) {
      for_each_influencing_entity(*decl.declared(), functor);
   }
}