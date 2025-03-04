#pragma once
#include "gcc_wrappers/type/record.h"
#include "codegen/func_pair.h"
#include "codegen/optional_value_pair.h"
#include "codegen/whole_struct_function_info.h"

namespace codegen {
   class whole_struct_function_dictionary;
}

namespace codegen::instructions::utils {
   struct generation_context {
      public:
         generation_context(whole_struct_function_dictionary& w) : whole_struct_functions(w) {}
      
      public:
         whole_struct_function_dictionary& whole_struct_functions;
         optional_value_pair state_ptr;
      
      protected:
         whole_struct_function_info _make_whole_struct_functions_for(gcc_wrappers::type::record) const;
      public:
         func_pair get_whole_struct_functions_for(gcc_wrappers::type::record) const;
   };
}