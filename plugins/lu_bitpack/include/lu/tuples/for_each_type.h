#pragma once
#include <tuple>
#include <type_traits>

namespace lu::tuples {
   template<typename Tuple, typename Functor>
   extern constexpr void for_each_type(Functor&& functor) {
      // IIFE to avoid the need for a helper function:
      []<std::size_t... I>(Functor&& functor, std::index_sequence<I...>) {
         (functor.template operator()<std::tuple_element_t<I, Tuple>>(), ...);
      }(
         std::forward<Functor>(functor),
         std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>{}
      );
   }
}