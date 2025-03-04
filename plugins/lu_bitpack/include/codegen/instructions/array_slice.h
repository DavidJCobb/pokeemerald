#pragma once
#include "codegen/instructions/base.h"
#include "codegen/decl_descriptor_pair.h"
#include "codegen/value_path.h"
#include "gcc_wrappers/decl/variable.h"
#include "gcc_wrappers/flow/simple_for_loop.h"

namespace codegen::instructions {
   //
   // Represents access into an array via a for-loop. This doesn't represent 
   // actually serializing an array element (because we may instead serialize 
   // members of that element); it only represents the for-loop itself.
   //
   class array_slice : public container {
      public:
         array_slice();
         
         static constexpr const type node_type = type::array_slice;
         virtual type get_type() const noexcept override { return node_type; };
         
         virtual expr_pair generate(const utils::generation_context&) const;
         
      public:
         struct {
            value_path value;
            size_t     start = 0;
            size_t     count = 0;
         } array;
         struct {
            struct {
               gcc_wrappers::decl::variable read; // VAR_DECL
               gcc_wrappers::decl::variable save; // VAR_DECL
            } variables;
            decl_descriptor_pair descriptors;
         } loop_index;
   };
}