#pragma once
#include "gcc_wrappers/node.h"

namespace gcc_wrappers {
   constexpr bool node::is_same(const node& o) const noexcept {
      return this->unwrap() == o.unwrap();
   }
   constexpr bool node::operator==(const node& o) const noexcept {
      return this->is_same(o);
   }
   
   constexpr const_tree node::unwrap() const {
      return this->_node;
   }
   constexpr tree node::unwrap()  {
      return this->_node;
   }
}