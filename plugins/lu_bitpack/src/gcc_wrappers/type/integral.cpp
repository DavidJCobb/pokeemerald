#include "gcc_wrappers/type/integral.h"
#include "gcc_wrappers/constant/integer.h"
#include "gcc_wrappers/_node_boilerplate-impl.define.h"

namespace gcc_wrappers::type {
   GCC_NODE_WRAPPER_BOILERPLATE(integral)
   
   size_t integral::bitcount() const {
      return TYPE_PRECISION(this->_node);
   }
   
   bool integral::is_signed() const {
      return !is_unsigned();
   }
   bool integral::is_unsigned() const {
      return TYPE_UNSIGNED(this->_node) != 0;
   }
   
   intmax_t integral::minimum_value() const {
      auto data = TYPE_MIN_VALUE(this->_node);
      if (TREE_CODE(data) == INTEGER_CST)
         return TREE_INT_CST_LOW(data);
      return 0;
   }
   uintmax_t integral::maximum_value() const {
      auto data = TYPE_MAX_VALUE(this->_node);
      if (TREE_CODE(data) == INTEGER_CST)
         return TREE_INT_CST_LOW(data);
      return 0;
   }
   
   integral integral::make_signed() const {
      return integral::wrap(signed_type_for(this->_node)); // tree.h
   }
   integral integral::make_unsigned() const {
      return integral::wrap(unsigned_type_for(this->_node)); // tree.h
   }
   
   bool integral::can_hold_value(const constant::integer c) const {
      return int_fits_type_p(c.unwrap(), this->unwrap());
   }
}