#pragma once
#include <memory>
#include <optional>
#include <unordered_map>
#include "lu/singleton.h"
#include "codegen/decl_descriptor.h"
#include "gcc_wrappers/decl/field.h"
#include "gcc_wrappers/decl/param.h"
#include "gcc_wrappers/decl/variable.h"
#include "gcc_wrappers/type/base.h"

namespace codegen {
   class decl_dictionary;
   class decl_dictionary : public lu::singleton<decl_dictionary> {
      protected:
         std::unordered_map<gcc_wrappers::decl::base, std::unique_ptr<decl_descriptor>> _data;
         
         // For transformations defined on types. If a type is known not to be 
         // transformed, then it's in here as an empty type so that if we see 
         // it again, we can check it faster. (We store optionals so that we 
         // can distinguish "never checked for" from "not transformed.")
         //
         // key, value = original, transformed
         std::unordered_map<gcc_wrappers::type::base, gcc_wrappers::type::optional_base> _transformations;
      
      public:
         // Creates a descriptor if one does not exist, or returns an existing 
         // descriptor.
         const decl_descriptor& describe(gcc_wrappers::decl::field);
         const decl_descriptor& describe(gcc_wrappers::decl::param);
         const decl_descriptor& describe(gcc_wrappers::decl::variable);
         
         // Create a descriptor for a pointer variable, and specify that what 
         // we are describing is the pointed-to value. Asserts that either no 
         // descriptor for the variable already exists, or the old descriptor 
         // has the same level of pointer indirection.
         //
         // Descriptors exist per-DECL, so you can't have two descriptors for 
         // the same DECL with different levels of indirection
         const decl_descriptor& dereference_and_describe(gcc_wrappers::decl::param, size_t how_many_times = 1);
         const decl_descriptor& dereference_and_describe(gcc_wrappers::decl::variable, size_t how_many_times = 1);
         
         const decl_descriptor* find_existing_descriptor(gcc_wrappers::decl::variable) const;
         
         gcc_wrappers::type::optional_base type_transforms_into(gcc_wrappers::type::base);
   };
}