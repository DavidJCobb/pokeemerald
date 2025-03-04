#pragma once
#include "gcc_wrappers/list_node.h"

namespace gcc_wrappers {
   // run the functor for each item's TREE_PURPOSE
   template<typename Functor>
   void list_node::for_each_key(Functor&& functor) {
      for(auto raw = this->_node; raw != NULL_TREE; raw = TREE_CHAIN(raw)) {
         auto key = optional_node(TREE_PURPOSE(raw));
         if constexpr (std::is_invocable_r_v<bool, Functor, optional_node>) {
            if (!functor(key))
               return;
         } else {
            functor(key);
         }
      }
   }
   
   // run the functor for each item's TREE_VALUE
   template<typename Functor>
   void list_node::for_each_value(Functor&& functor) {
      for(auto raw = this->_node; raw != NULL_TREE; raw = TREE_CHAIN(raw)) {
         auto value = optional_node(TREE_VALUE(raw));
         if constexpr (std::is_invocable_r_v<bool, Functor, optional_node>) {
            if (!functor(value))
               return;
         } else {
            functor(value);
         }
      }
   }
   
   // run the functor for each item's TREE_PURPOSE and TREE_VALUE
   template<typename Functor>
   void list_node::for_each_kv_pair(Functor&& functor) {
      for(auto raw = this->_node; raw != NULL_TREE; raw = TREE_CHAIN(raw)) {
         auto key   = optional_node(TREE_PURPOSE(raw));
         auto value = optional_node(TREE_VALUE(raw));
         if constexpr (std::is_invocable_r_v<bool, Functor, optional_node, optional_node>) {
            if (!functor(key, value))
               return;
         } else {
            functor(key, value);
         }
      }
   }
}