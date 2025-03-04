#pragma once
#include <type_traits>
#include "gcc_wrappers/type/container.h"

namespace gcc_wrappers::type {
   template<typename Functor>
   void container::for_each_field(Functor&& functor) const {
      for(auto item : this->member_chain()) {
         if (item.code() != FIELD_DECL)
            continue;
         auto decl = item.as<decl::field>();
         if constexpr (std::is_invocable_r_v<bool, Functor, decl::field>) {
            if (!functor(decl))
               break;
         } else {
            functor(decl);
         }
      }
   }
   
   template<typename Functor>
   void container::for_each_referenceable_field(Functor&& functor) const {
      for(auto item : this->member_chain()) {
         if (item.code() != FIELD_DECL)
            continue;
         
         if (DECL_VIRTUAL_P(item.unwrap()))
            //
            // This is the VTBL pointer. Skip it.
            //
            continue;
         
         if (DECL_NAME(item.unwrap()) == NULL_TREE) {
            //
            // This data member is unnamed. If it's a `container` itself, then 
            // recurse over its fields.
            //
            auto type_node = TREE_TYPE(item.unwrap());
            if (container::raw_node_is(type_node))
               container::wrap(type_node).for_each_referenceable_field(functor);
            continue;
         }
         
         auto decl = item.as<decl::field>();
         if constexpr (std::is_invocable_r_v<bool, Functor, decl::field>) {
            if (!functor(decl))
               break;
         } else {
            functor(decl);
         }
      }
   }
   
   template<typename Functor>
   void container::for_each_static_data_member(Functor&& functor) const {
      for(auto item : this->member_chain()) {
         if (item.code() != VAR_DECL)
            continue;
         auto decl = item.as<decl::variable>();
         if constexpr (std::is_invocable_r_v<bool, Functor, decl::variable>) {
            if (!functor(decl))
               break;
         } else {
            functor(decl);
         }
      }
   }
}