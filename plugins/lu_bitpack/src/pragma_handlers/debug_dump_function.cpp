#include "pragma_handlers/debug_dump_function.h"
#include <iostream>
#include <string_view>

#include <tree.h>
#include <c-family/c-common.h> // lookup_name

#include <print-tree.h> // debug_tree

#include "gcc_wrappers/decl/function.h"
#include "gcc_wrappers/decl/param.h"
#include "gcc_wrappers/type/function.h"
#include "gcc_wrappers/identifier.h"
namespace gw {
   using namespace gcc_wrappers;
}

namespace pragma_handlers {
   extern void debug_dump_function(cpp_reader* reader) {
      constexpr const char* this_pragma_name = "#pragma lu_bitpack debug_dump_function";
      
      gw::optional_identifier name_opt;
      {
         location_t loc;
         tree       data;
         auto token_type = pragma_lex(&data, &loc);
         if (token_type != CPP_NAME) {
            std::cerr << "error: " << this_pragma_name << ": not a valid identifier\n";
            return;
         }
         name_opt = data;
      }
      
      auto raw = lookup_name(name_opt.unwrap());
      if (raw == NULL_TREE) {
         std::cerr << "error: " << this_pragma_name << ": identifier " << name_opt->name() << " not found\n";
         return;
      }
      if (TREE_CODE(raw) != FUNCTION_DECL) {
         std::cerr << "error: " << this_pragma_name << ": identifier " << name_opt->name() << " is something other than a function\n";
         return;
      }
      
      auto decl = gw::decl::function::wrap(raw);
      auto type = decl.function_type();
      
      std::cerr << "Dumping information for function...\n";
      std::cerr << " - Unqualified name: " << decl.name() << '\n';
      std::cerr << " - Type/signature: " << type.pretty_print() << '\n';
      if (decl.has_body()) {
         auto retn = decl.result_variable();
         //
         // `retn` is basically an implicit variable, and assigning to it 
         // is how a function returns a value. Think of it like Result in 
         // some variants of Pascal/Delphi.
         //
         std::cerr << " - Return type: ";
         std::cerr << retn.value_type().pretty_print();
         std::cerr << '\n';
      }
      
      std::cerr << " - Arguments:\n";
      if (type.is_unprototyped()) {
         std::cerr << "    - <none - unprototyped>\n";
      } else {
         size_t size    = type.fixed_argument_count();
         bool   varargs = type.is_varargs();
         if (size == 0) {
            if (!varargs) {
               std::cerr << "    - <none>\n";
            }
         } else {
            for(size_t i = 0; i < size; ++i) {
               std::cerr << "    - ";
               
               auto pd   = decl.nth_parameter(i);
               auto name = pd.name();
               if (name.empty())
                  std::cerr << "<unnamed>";
               else
                  std::cerr << name;
               std::cerr << '\n';
               
               std::cerr << "       - Type (via param): ";
               std::cerr << pd.value_type().pretty_print();
               std::cerr << '\n';
               
               std::cerr << "       - Type (via function type): ";
               std::cerr << type.nth_argument_type(i).pretty_print();
               std::cerr << '\n';
            }
         }
         if (varargs) {
            std::cerr << "    - <varargs...>\n";
         }
      }
      
      debug_tree(decl.unwrap());
      if (decl.has_body()) {
         std::cerr << " - Dumping function body...\n";
         debug_tree(DECL_SAVED_TREE(decl.unwrap()));
      }
      std::cerr << " - Done.\n\n";
   }
}