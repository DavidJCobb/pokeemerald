#pragma once
#include <type_traits>
#include <variant>

namespace lu::variants {
   namespace impl {
      template<typename Variant, typename Desired>
      struct _contains_type;

      template<typename Desired, typename... Types>
      struct _contains_type<std::variant<Types...>, Desired> {
         static constexpr bool value = (std::is_same_v<Types, Desired> || ...);
      };
   }

   template<typename Variant, typename T>
   constexpr bool contains_type = impl::_contains_type<std::decay_t<Variant>, T>::value;
}