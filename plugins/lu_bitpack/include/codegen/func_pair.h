#pragma once
#include "gcc_wrappers/decl/function.h"

namespace codegen {
   struct func_pair;
   
   struct optional_func_pair {
      gcc_wrappers::decl::optional_function read;
      gcc_wrappers::decl::optional_function save;
      
      optional_func_pair& operator=(const func_pair&) noexcept;
   };
   
   struct func_pair {
      func_pair(const func_pair&) = default;
      func_pair(func_pair&&) noexcept = default;
      
      func_pair(const optional_func_pair& pair);
      func_pair(gcc_wrappers::decl::function read, gcc_wrappers::decl::function save);
      
      gcc_wrappers::decl::function read;
      gcc_wrappers::decl::function save;
   };
}