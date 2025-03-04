#pragma once
#include <concepts>
#include <tuple>
#include <version>
#include "lu/tuples/contains_type.h"
#include "lu/tuples/contains_type_matching_predicate.h"
#include "lu/tuples/filter_types.h"
#include "lu/tuples/find_first_matching_type.h"
#include "lu/tuples/first_index_of_type.h"
#include "lu/tuples/transform_types.h"
#include "lu/tuples/unpack_types_into.h"

namespace lu::type_containers {
   template<typename... Types>
   class array;
   
   namespace impl::_array {
      template<auto Functor, typename... Types> concept functor_returns_bool = requires {
         { (Functor.template operator()<Types>() || ...) } -> std::same_as<bool>;
      };
   }
   
   template<typename... Types>
   class array {
      public:
         static constexpr const size_t index_of_none = (size_t)-1;

         // This container cannot be instantiated; it is a compile-time thing only.
         array() = delete;
         array(const array&) = delete;
         array(array&&) = delete;
         ~array() = delete;

      public:
         using as_tuple = std::tuple<Types...>;

         template<std::size_t N> requires (N < sizeof...(Types))
         using nth_type = typename std::tuple_element<N, as_tuple>::type;

         static constexpr size_t count = sizeof...(Types);
         
         static
         #if __cpp_consteval >= 201811L
            consteval
         #else
            constexpr
         #endif
         size_t size() noexcept { return count; }

         template<typename T>
         static constexpr bool contains = tuples::contains_type<as_tuple, T>;
         
         template<typename T>
         static constexpr size_t index_of = tuples::first_index_of_type<as_tuple, T>;

         template<auto Functor>
         static constexpr bool contains_matching_type = tuples::contains_type_matching_predicate<as_tuple, Functor>;
         
         template<auto Functor>
         static constexpr size_t index_of_matching_type = tuples::find_first_matching_type<as_tuple, Functor>;
         
         template<auto Functor>
         using get_matching_type = nth_type<index_of_matching_type<Functor>>;

         template<auto Functor, typename... Args>
         static constexpr void for_each(Args&&... args) {
            (Functor.template operator()<Types>(std::forward<Args>(args)...), ...);
         }

         template<typename Functor, typename... Args>
         static constexpr void for_each(Functor&& f, Args&&... args) {
            (f.template operator()<Types>(std::forward<Args>(args)...), ...);
         }
         
         template<auto Functor, typename... Args>
         static constexpr bool for_each_until_false(Args&&... args) {
            return (Functor.template operator()<Types>(std::forward<Args>(args)...) && ...);
         }
         template<typename Functor, typename... Args>
         static constexpr bool for_each_until_false(Functor&& f, Args&&... args) {
            return (f.template operator()<Types>(std::forward<Args>(args)...) && ...);
         }
         
         template<auto Functor, typename... Args>
         static constexpr bool for_each_until_true(Args&&... args) {
            return (Functor.template operator()<Types>(std::forward<Args>(args)...) || ...);
         }
         template<typename Functor, typename... Args>
         static constexpr bool for_each_until_true(Functor&& f, Args&&... args) {
            return (f.template operator()<Types>(std::forward<Args>(args)...) || ...);
         }
         
         template<typename Result, auto Functor>
            #ifndef __INTELLISENSE__
            requires requires(Result prev) {
               { Functor.template operator()<nth_type<0>>(prev) } -> std::convertible_to<Result>;
            }
            #endif
         static constexpr Result reduce(Result prev = {}) {
            return ((prev = Functor.template operator()<Types>(prev)), ...);
         }

         template<auto Functor>
         using filter_types = lu::tuples::unpack_types_into<tuples::filter_types<as_tuple, Functor>, ::lu::type_containers::array>;
         
         template<template<typename> typename Transform>
         using transform_types = lu::tuples::unpack_types_into<tuples::transform_types<as_tuple, Transform>, ::lu::type_containers::array>;
   };
}