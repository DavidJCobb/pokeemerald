#include "gcc_wrappers/environment/c_family/language.h"
#include <gcc-plugin.h>
#include <c-family/c-common.h> // c_language and the c_language_kind enum

namespace gcc_wrappers::environment::c_family {
   extern language current_language() {
      switch (c_language) {
         case clk_c:      return language::c;
         case clk_cxx:    return language::cpp;
         case clk_objc:   return language::objective_c;
         case clk_objcxx: return language::objective_cpp;
      }
      return language::none;
   }
   
   extern bool language_is_cpp_ish() {
      return c_dialect_cxx();
   }
   extern bool language_is_objective() {
      return c_dialect_objc();
   }
}