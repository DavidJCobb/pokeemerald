#include "gcc_wrappers/constant/floating_point.h"
#include "gcc_wrappers/constant/integer.h"
#include "gcc_wrappers/_node_boilerplate-impl.define.h"
#include <real.h> // comparisons

namespace gcc_wrappers::constant {
   GCC_NODE_WRAPPER_BOILERPLATE(floating_point)
   
   HOST_WIDE_INT floating_point::_to_host_wide_int() const {
      return real_to_integer(TREE_REAL_CST_PTR(this->_node));
   }
   
   /*explicit*/ floating_point::floating_point(type::floating_point t, integer n) {
      this->_node = build_real_from_int_cst(t.unwrap(), n.unwrap());
   }

   /*static*/ floating_point floating_point::from_string(type::floating_point type, lu::strings::zview str) {
      REAL_VALUE_TYPE raw;
      /*int overflow =*/ real_from_string(&raw, str.c_str());
      
      // TODO: Set TREE_OVERFLOW(node) based on `overflow != 0`?
      return floating_point::wrap(build_real(type.unwrap(), raw));
   }
   
   /*static*/ floating_point floating_point::make_infinity(type::floating_point type, bool sign) {
      REAL_VALUE_TYPE raw;
      #if GCCPLUGIN_VERSION_MAJOR >= 13
         real_inf(&raw, sign);
      #else
         real_inf(&raw);
         raw.sign = sign;
      #endif
      return floating_point::wrap(build_real(type.unwrap(), raw));
   }

   bool floating_point::is_zero() const {
      return real_zerop(this->_node);
   }
   bool floating_point::is_one() const {
      return real_onep(this->_node);
   }
   bool floating_point::is_negative_one() const {
      return real_minus_onep(this->_node);
   }
   
   // checks from real.h
   bool floating_point::is_any_infinity() const {
      return real_isinf(TREE_REAL_CST_PTR(this->_node));
   }
   bool floating_point::is_signed_infinity(bool negative) const {
      auto* raw = TREE_REAL_CST_PTR(this->_node);
      #if GCCPLUGIN_VERSION_MAJOR >= 13
         return real_isinf(raw, negative);
      #else
         if (!real_isinf(raw))
            return false;
         return raw->sign == negative;
      #endif
   }
   bool floating_point::is_nan() const {
      return real_isnan(TREE_REAL_CST_PTR(this->_node));
   }
   bool floating_point::is_signalling_nam() const {
      return real_issignaling_nan(TREE_REAL_CST_PTR(this->_node));
   }
   
   bool floating_point::is_negative() const {
      return real_isneg(TREE_REAL_CST_PTR(this->_node));
   }
   bool floating_point::is_negative_zero() const {
      return real_isnegzero(TREE_REAL_CST_PTR(this->_node));
   }
   
   bool floating_point::is_bitwise_identical(const floating_point other) const {
      return real_identical(TREE_REAL_CST_PTR(this->_node), TREE_REAL_CST_PTR(other._node));
   }
   
   std::string floating_point::to_string() const {
      std::string result;
      result.resize(64);
      real_to_decimal(
         result.data(),
         TREE_REAL_CST_PTR(this->_node),
         result.size(),
         0,   // max digits; 0 for no limit
         true // crop trailing zeroes
      );
      {
         size_t i = result.find_first_of('\0');
         if (i != std::string::npos)
            result.resize(i);
      }
      return result;
   }
   
   bool floating_point::operator<(const floating_point& other) const {
      return real_less(TREE_REAL_CST_PTR(this->_node), TREE_REAL_CST_PTR(other._node));
   }
   bool floating_point::operator==(const floating_point& other) const {
      return real_equal(TREE_REAL_CST_PTR(this->_node), TREE_REAL_CST_PTR(other._node));
   }
}