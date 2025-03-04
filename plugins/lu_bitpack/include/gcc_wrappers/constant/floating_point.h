#pragma once
#include <optional>
#include <string>
#include <type_traits>
#include "lu/strings/zview.h"
#include "gcc_wrappers/constant/base.h"
#include "gcc_wrappers/type/floating_point.h"
#include "gcc_headers/plugin-version.h"
#include "gcc_wrappers/_node_boilerplate.define.h"

namespace gcc_wrappers::constant {
   class integer;
}

namespace gcc_wrappers::constant {
   class floating_point : public base {
      public:
         static bool raw_node_is(tree t) {
            return TREE_CODE(t) == REAL_CST;
         }
         GCC_NODE_WRAPPER_BOILERPLATE(floating_point)
         
      protected:
         HOST_WIDE_INT _to_host_wide_int() const;
      
      public:
         explicit floating_point(type::floating_point, integer);
         
         static floating_point from_string(type::floating_point, lu::strings::zview);
         
         static floating_point make_infinity(type::floating_point, bool sign = false);
         
         bool is_zero() const; // real_zerop
         bool is_one() const; // real_onep
         bool is_negative_one() const; // real_minus_onep
         
         bool is_any_infinity() const;
         bool is_signed_infinity(bool negative) const;
         bool is_nan() const;
         bool is_signalling_nam() const;
         
         bool is_negative() const;
         bool is_negative_zero() const;
         
         bool is_bitwise_identical(const floating_point) const;
         
         std::string to_string() const;
         
         template<typename Integral> requires std::is_integral_v<Integral>
         Integral to_integer() const {
            return (Integral) _to_host_wide_int();
         }
         
         bool operator<(const floating_point&) const;
         bool operator==(const floating_point&) const;
   };
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(floating_point);
}

#include "gcc_wrappers/_node_boilerplate.undef.h"