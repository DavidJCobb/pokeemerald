#pragma once
#include <tuple>
#include <type_traits>

namespace lu::tuples {
   namespace impl {
      template<typename Tuple, typename Desired>
      struct _first_index_of_type;

      template<typename Desired, typename... Types>
      struct _first_index_of_type<std::tuple<Types...>, Desired> {
         static constexpr size_t value = [](){
            size_t i = 0;
            ((std::is_same_v<Types, Desired> || (++i, false)) || ...);
            if (i >= sizeof...(Types))
               return (size_t)-1;
            return i;
         }();
      };
   }

   template<typename Tuple, typename T>
   constexpr size_t first_index_of_type = impl::_first_index_of_type<Tuple, T>::value;
}