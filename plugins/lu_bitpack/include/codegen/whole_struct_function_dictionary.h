#pragma once
#include <memory>
#include <unordered_map>
#include "codegen/func_pair.h"
#include "codegen/whole_struct_function_info.h"
#include "gcc_wrappers/type/base.h"

namespace codegen {
   class whole_struct_function_dictionary {
      protected:
         std::unordered_map<gcc_wrappers::type::base, whole_struct_function_info> _functions;
         
      public:
         optional_func_pair get_functions_for(gcc_wrappers::type::base) const;
         const instructions::base* get_instructions_for(gcc_wrappers::type::base) const;
         
         // Asserts that functions don't already exist for the given struct type.
         void add_functions_for(gcc_wrappers::type::base, whole_struct_function_info&&);
         
         template<typename Functor>
         void for_each(Functor&& functor) const {
            for(auto& pair : this->_functions) {
               functor(pair.first, pair.second);
            }
         }
   };
}