#include "gcc_helpers/c/at_file_scope.h"

// Declared in `c/c-tree.h`, which is not included in the plug-in headers 
// as of GCC 11.4.0.
extern bool global_bindings_p(void);

namespace gcc_helpers::c {
   extern bool at_file_scope() {
      return global_bindings_p();
   }
}