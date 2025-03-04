#include "gcc_wrappers/type/pointer.h"
#include <target.h> // `targetm`
#include "gcc_wrappers/_node_boilerplate-impl.define.h"

namespace gcc_wrappers::type {
   GCC_NODE_WRAPPER_BOILERPLATE(pointer)
   
   bool pointer::compatible_address_space_with(pointer other) const {
      auto target_a = this->remove_pointer();
      auto target_b = other.remove_pointer();
      auto as_a = TYPE_ADDR_SPACE(target_a.unwrap());
      auto as_b = TYPE_ADDR_SPACE(target_b.unwrap());
      if (as_a == as_b)
         return true;
      
      //
      // See `addr_space_superset` in `c/c-typeck.cc`
      //
      if (targetm.addr_space.subset_p(as_a, as_b))
         return true;
      if (targetm.addr_space.subset_p(as_b, as_a))
         return true;
      return false;
   }
   bool pointer::same_target_and_address_space_as(pointer other) const {
      if (!this->compatible_address_space_with(other))
         return false;
      auto main_a = this->remove_pointer().main_variant();
      auto main_b = other.remove_pointer().main_variant();
      
      auto strip_a  = base::wrap(strip_array_types(main_a.unwrap()));
      auto strip_b  = base::wrap(strip_array_types(main_b.unwrap()));
      //
      if (strip_a.is_atomic())
         main_a = main_a.with_all_qualifiers_stripped().add_atomic();
      if (strip_b.is_atomic())
         main_b = main_b.with_all_qualifiers_stripped().add_atomic();
      
      return main_a == main_b;
   }
}