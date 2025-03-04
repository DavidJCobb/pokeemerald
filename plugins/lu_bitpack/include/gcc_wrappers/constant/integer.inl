#pragma once
#include "gcc_wrappers/constant/integer.h"

namespace gcc_wrappers {
   namespace constant {
      template<typename Integer>
      std::optional<Integer> integer::value() const requires (std::is_integral_v<Integer> && std::is_arithmetic_v<Integer>) {
         using intermediate_type = std::conditional_t<
            std::is_signed_v<Integer>,
            host_wide_int_type,
            host_wide_uint_type
         >;
         
         std::optional<intermediate_type> opt;
         if constexpr (std::is_signed_v<Integer>) {
            opt = try_value_signed();
         } else {
            opt = try_value_unsigned();
         }
         if (!opt.has_value())
            return {};
         
         intermediate_type v = *opt;
         if constexpr (!std::is_same_v<Integer, intermediate_type>) {
            if constexpr (std::is_signed_v<Integer>) {
               if (v < std::numeric_limits<Integer>::lowest())
                  return {};
            }
            if (v > std::numeric_limits<Integer>::max())
               return {};
         }
         return (Integer) v;
      }
   }
}