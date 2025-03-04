#pragma once
#include "gcc_wrappers/chain.h"

namespace gcc_wrappers {
   template<typename Functor>
   void chain::for_each(Functor&& functor) {
      if (!this->_node)
         return;
      size_t i = 0;
      for(auto raw = this->_node->unwrap(); raw != NULL_TREE; raw = TREE_CHAIN(raw)) {
         auto n = node::wrap(raw);
         if constexpr (std::is_invocable_v<Functor, node, size_t>) {
            if constexpr (std::is_invocable_r_v<bool, Functor, node, size_t>) {
               if (!functor(n, i))
                  return;
            } else {
               functor(n, i);
            }
            ++i;
         } else {
            if constexpr (std::is_invocable_r_v<bool, Functor, node>) {
               if (!functor(n))
                  return;
            } else {
               functor(n);
            }
         }
      }
   }
}