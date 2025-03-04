#pragma once
#include <tuple>

namespace lu::tuples {
   namespace impl {
      template<typename Tuple, auto Predicate>
      struct _contains_type_matching_predicate;

      template<auto Predicate, typename... Types>
         requires requires {
            { (Predicate.template operator()<Types>() || ...) } -> std::same_as<bool>;
         }
      struct _contains_type_matching_predicate<std::tuple<Types...>, Predicate> {
         static constexpr const bool value = (Predicate.template operator()<Types>() || ...);
      };

      template<auto Predicate>
      struct _contains_type_matching_predicate<std::tuple<>, Predicate> {
         static constexpr const bool value = false;
      };
   }
   
   /*
   
      Usage:
     
         lu::tuples::contains_type_matching_predicate<
            tuple,
            []<typename Current>() -> bool {
               return false; // do your checks here; return true on match
            }
         >;
   
   */
   template<typename Tuple, auto Predicate>
   constexpr bool contains_type_matching_predicate = impl::_contains_type_matching_predicate<Tuple, Predicate>::value;
}