#include "gcc_wrappers/constant/integer.h"
#include "gcc_wrappers/_node_boilerplate-impl.define.h"

namespace gcc_wrappers::constant {
   GCC_NODE_WRAPPER_BOILERPLATE(integer)
   
   /*explicit*/ integer::integer(type::integral t, int n) {
      this->_node = build_int_cst(t.unwrap(), n);
   }
   
   type::integral integer::value_type() const {
      return type::integral::wrap(TREE_TYPE(this->_node));
   }
   
   std::optional<integer::host_wide_int_type>  integer::try_value_signed() const {
      if (!tree_fits_shwi_p(this->_node))
         return {};
      return (host_wide_int_type) TREE_INT_CST_LOW(this->_node);
   }
   std::optional<integer::host_wide_uint_type> integer::try_value_unsigned() const {
      if (!tree_fits_uhwi_p(this->_node))
         return {};
      return (host_wide_uint_type) TREE_INT_CST_LOW(this->_node);
   }
   
   int integer::sign() const {
      return tree_int_cst_sgn(this->_node);
   }
            
   bool integer::operator<(const integer& other) const {
      //
      // NOTE: This doesn't handle the case of the values having types of 
      // different signedness.
      //
      return tree_int_cst_lt(this->_node, other._node);
   }
   bool integer::operator==(const integer& other) const {
      return tree_int_cst_equal(this->_node, other._node);
   }
   
   bool integer::operator==(host_wide_uint_type v) const {
      return compare_tree_int(this->_node, v);
   }
}