#pragma once
#include <array>
#include <string>
#include <string_view>
#include "gcc_wrappers/decl/function.h"
#include "gcc_wrappers/type/base.h"
#include "gcc_wrappers/type/function.h"

namespace gcc_helpers {
   template<size_t ArgCount>
   std::string stringify_function_signature(
      bool                     is_varargs,
      gcc_wrappers::type::base return_type,
      std::string_view         function_name,
      std::array<gcc_wrappers::type::base, ArgCount>& args
   ) {
      std::string str;
      str += return_type.pretty_print();
      str += ' ';
      str += std::string(function_name);
      str += '(';
      for(size_t i = 0; i < ArgCount; ++i) {
         if (i > 0)
            str += ", ";
         str += args[i].pretty_print();
      }
      if (is_varargs) {
         if constexpr (ArgCount > 0)
            str += ", ";
         str += "...";
      }
      str += ')';
      return str;
   }
   
   extern std::string stringify_function_signature(gcc_wrappers::decl::function);
   extern std::string stringify_function_signature(gcc_wrappers::type::function, std::string_view function_name);
}