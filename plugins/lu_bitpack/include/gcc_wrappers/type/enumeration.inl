#pragma once
#include <type_traits>
#include "gcc_wrappers/type/enumeration.h"

namespace gcc_wrappers::type {
   template<typename Functor>
   void enumeration::for_each_enum_member(Functor&& functor) const {
      for(auto item = TYPE_VALUES(_node); item != NULL_TREE; item = TREE_CHAIN(item)) {
         const auto m = member{
            .name  = std::string_view(IDENTIFIER_POINTER(TREE_PURPOSE(item))),
            .value = (intmax_t)TREE_INT_CST_LOW(TREE_VALUE(item)),
         };
         if constexpr (std::is_invocable_r_v<bool, Functor, const member&>) {
            if (!functor(m))
               break;
         } else {
            functor(m);
         }
      }
   }
}