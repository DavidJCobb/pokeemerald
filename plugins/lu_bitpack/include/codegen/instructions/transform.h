#pragma once
#include "codegen/instructions/base.h"
#include "codegen/decl_descriptor_pair.h"
#include "codegen/value_path.h"
#include "gcc_wrappers/decl/variable.h"
#include "gcc_wrappers/type/base.h"

namespace codegen::instructions {
   class transform : public container {
      public:
         transform(const std::vector<gcc_wrappers::type::base>&);
      
         static constexpr const type node_type = type::transform;
         virtual type get_type() const noexcept override { return node_type; };
         
         virtual expr_pair generate(const utils::generation_context&) const;
         
      public:
         value_path to_be_transformed_value;
         
         std::vector<gcc_wrappers::type::base> types;
         
         // We pre-create VAR_DECLs for the final transformed value, so that 
         // other nodes can refer to those variables in `value_path`s.
         struct {
            struct {
               gcc_wrappers::decl::variable read;
               gcc_wrappers::decl::variable save;
            } variables;
            decl_descriptor_pair descriptors;
         } transformed;
         
      public:
         bool is_probably_split() const;
   };
}