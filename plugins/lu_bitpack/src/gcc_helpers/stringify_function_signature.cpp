#include "gcc_helpers/stringify_function_signature.h"
namespace gw {
   using namespace gcc_wrappers;
}

namespace gcc_helpers {
   extern std::string stringify_function_signature(gw::decl::function decl) {
      auto type = decl.function_type();
      return stringify_function_signature(type, decl.name());
   }
   extern std::string stringify_function_signature(gw::type::function type, std::string_view function_name) {
      std::string str;
      str += type.return_type().pretty_print();
      str += ' ';
      str += function_name;
      str += '(';
      if (!type.is_unprototyped()) {
         bool any = false;
         type.for_each_argument_type([&any, &str](gw::type::base arg_type, size_t i) {
            any = true;
            if (i > 0)
               str += ", ";
            str += arg_type.pretty_print();
         });
         if (type.is_varargs()) {
            if (any)
               str += ", ";
            str += "...";
         }
      }
      str += ')';
      return str;
   }
}