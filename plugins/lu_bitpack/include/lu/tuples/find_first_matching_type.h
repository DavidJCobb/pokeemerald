#pragma once
#include <concepts>
#include <tuple>

namespace lu::tuples {
   namespace impl {
      template<typename Tuple, auto Predicate>
      struct _find_first_matching_type;

      template<auto Predicate, typename... Types> requires requires {
         { (Predicate.template operator()<Types>() || ...) } -> std::same_as<bool>;
      }
      struct _find_first_matching_type<std::tuple<Types...>, Predicate> {
         static constexpr auto value = []() consteval {
            size_t i = 0;
            ((Predicate.template operator()<Types>() || (++i, false)) || ...);
            return i < sizeof...(Types) ? i : (size_t)-1;
         }();
      };

      template<auto Predicate>
      struct _find_first_matching_type<std::tuple<>, Predicate> {
         static constexpr size_t value = (size_t)-1;
      };
   }

   /*
   
      Usage:
     
         constexpr const size_t i = lu::tuples::find_first_matching_type<
            tuple,
            []<typename Current>() -> bool {
               return false; // do your checks here; return true on match
            }
         >;
         if constexpr (i != (size_t)-1) {
            // found
         }
   
   */
   template<typename Tuple, auto Predicate>
   constexpr size_t find_first_matching_type = impl::_find_first_matching_type<Tuple, Predicate>::value;
}