#pragma once
#include <limits>
#include <vector>
#include "bitpacking/data_options.h"
#include "gcc_wrappers/decl/field.h"
#include "gcc_wrappers/decl/param.h"
#include "gcc_wrappers/decl/variable.h"
#include "gcc_wrappers/type/base.h"

namespace codegen {
   class decl_descriptor {
      public:
         // Used in the `array.extents` list to indicate a variable-length array.
         static constexpr const size_t vla_extent = std::numeric_limits<size_t>::max();
      
      protected:
         // assumes `options` is already set
         void _compute_types();
      
      public:
         decl_descriptor(gcc_wrappers::decl::field);
         decl_descriptor(gcc_wrappers::decl::param,    size_t dereference_count = 0);
         decl_descriptor(gcc_wrappers::decl::variable, size_t dereference_count = 0);
         
      public:
         gcc_wrappers::decl::base_value decl;
         bitpacking::data_options       options;
         struct {
            // Value type of the `decl` that we're wrapping.
            gcc_wrappers::type::base basic_type;
            
            // If the `decl` we're wrapping is of an array type, then this will be 
            // the innermost element type.
            gcc_wrappers::type::optional_base innermost;
            
            // Value type that we're going to write into the bitstream. For strings, 
            // this will be an array; for transformed values, this will be the final 
            // transformed type; for everything else, it will be the same as the 
            // `innermost` type.
            gcc_wrappers::type::optional_base serialized;
            
            // In transformation order: if A -> B and B -> C, this is [A, B, C].
            std::vector<gcc_wrappers::type::base> transformations;
         } types;
         struct {
            std::vector<size_t> extents; // for convenience: array extents leading to `serialized`
         } array;
         struct {
            // If non-zero, then the DECL is a pointer, and this descriptor describes 
            // a pointed-to value that one can access by dereferencing the DECL this 
            // many times. Only valid for VAR_DECLs and PARM_DECLs.
            size_t dereference_count = 0;
         } variable;
         bool has_any_errors = false;
         
         // Gets the members of the `serialized` type, as decl descriptors.
         // Asserts that said type is a struct or union. Members may be 
         // omitted from bitpacking; the caller must check.
         std::vector<const decl_descriptor*> members_of_serialized() const;
         
         size_t serialized_type_size_in_bits() const;
         
         size_t unpacked_single_size_in_bits() const;
         
         bool is_or_contains_defaulted() const;
   };
}