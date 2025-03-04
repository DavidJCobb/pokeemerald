#include "gcc_wrappers/type/fixed_point.h"
#include "gcc_wrappers/_node_boilerplate-impl.define.h"

namespace gcc_wrappers::type {
   GCC_NODE_WRAPPER_BOILERPLATE(fixed_point)
   
   size_t fixed_point::fractional_bitcount() const {
      return TYPE_FBIT(this->_node);
   }
   size_t fixed_point::integral_bitcount() const {
      return TYPE_IBIT(this->_node);
   }
   size_t fixed_point::total_bitcount() const {
      return TYPE_PRECISION(this->_node);
   }
   
   bool fixed_point::is_saturating() const {
      return TYPE_SATURATING(this->_node) != 0;
   }
   bool fixed_point::is_signed() const {
      return !is_unsigned();
   }
   bool fixed_point::is_unsigned() const {
      return TYPE_UNSIGNED(this->_node) != 0;
   }
   
   intmax_t fixed_point::minimum_value() const {
      auto data = TYPE_MIN_VALUE(this->_node);
      if (TREE_CODE(data) == INTEGER_CST)
         return TREE_INT_CST_LOW(data);
      return 0;
   }
   uintmax_t fixed_point::maximum_value() const {
      auto data = TYPE_MAX_VALUE(this->_node);
      if (TREE_CODE(data) == INTEGER_CST)
         return TREE_INT_CST_LOW(data);
      return 0;
   }
   
   fixed_point fixed_point::make_signed() const {
      return fixed_point::wrap(signed_type_for(this->_node)); // tree.h
   }
   fixed_point fixed_point::make_unsigned() const {
      return fixed_point::wrap(unsigned_type_for(this->_node)); // tree.h
   }
}