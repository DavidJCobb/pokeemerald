#pragma once
#include <string>
#include "gcc_wrappers/decl/base_value.h"
#include "gcc_wrappers/_node_boilerplate.define.h"

namespace gcc_wrappers::type {
   class container;
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(container);
}

namespace gcc_wrappers::decl {
   class field : public base_value {
      public:
         static bool raw_node_is(tree t) {
            return TREE_CODE(t) == FIELD_DECL;
         }
         GCC_NODE_WRAPPER_BOILERPLATE(field)
         
      public:
         type::optional_container member_of() const; // RECORD_TYPE or UNION_TYPE
         
         type::base value_type() const; // override, to handle bitfields
         
         size_t offset_in_bytes() const;
         size_t offset_in_bits() const;
         size_t size_in_bits() const;
         
         // Indicates whether this field is a bitfield. If so, 
         // `size_in_bits` is the field width.
         bool is_bitfield() const;
         size_t bitfield_offset_within_unit() const;
         type::optional_base bitfield_type() const; // return the type variant created for a bitfield
         
         bool is_non_addressable() const; // DECL_NONADDRESSABLE_P
         bool is_padding() const;
         
         std::string pretty_print() const;
         
         bool is_class_vtbl_pointer() const;
   };
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(field);
}

#include "gcc_wrappers/_node_boilerplate.undef.h"