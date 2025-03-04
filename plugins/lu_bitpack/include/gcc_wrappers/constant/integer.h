#pragma once
#include <optional>
#include <type_traits>
#include "gcc_wrappers/constant/base.h"
#include "gcc_wrappers/type/integral.h"
#include "gcc_wrappers/_node_boilerplate.define.h"

namespace gcc_wrappers::constant {
   class integer : public base {
      public:
         static bool raw_node_is(tree t) {
            return TREE_CODE(t) == INTEGER_CST;
         }
         GCC_NODE_WRAPPER_BOILERPLATE(integer)
         
         using host_wide_int_type  = HOST_WIDE_INT;
         using host_wide_uint_type = std::make_unsigned_t<host_wide_int_type>;
         
      public:
         explicit integer(type::integral, int);
         
         type::integral value_type() const;
         
         std::optional<host_wide_int_type>  try_value_signed() const;
         std::optional<host_wide_uint_type> try_value_unsigned() const;
         
         // Returns empty if the value is not representable in the given integer type.
         template<typename Integer>
         std::optional<Integer> value() const requires (std::is_integral_v<Integer> && std::is_arithmetic_v<Integer>);
         
         int sign() const;
         
         bool operator<(const integer&) const;
         bool operator==(const integer&) const;
         
         bool operator==(host_wide_uint_type) const;
   };
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(integer);
}

#include "gcc_wrappers/_node_boilerplate.undef.h"
#include "gcc_wrappers/constant/integer.inl"