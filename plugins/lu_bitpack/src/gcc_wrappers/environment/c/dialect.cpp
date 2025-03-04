#include "gcc_wrappers/environment/c/dialect.h"
#include <gcc-plugin.h>
#include <c-family/c-common.h>
#include "gcc_headers/plugin-version.h"

namespace gcc_wrappers::environment::c {
   extern dialect current_dialect() {
      if (!flag_isoc94)
         return dialect::c89;
      if (!flag_isoc99)
         return dialect::c89_amendment_1;
      if (!flag_isoc11)
         return dialect::c99;
      if (
         #if GCCPLUGIN_VERSION_MAJOR >= 14 && GCCPLUGIN_VERSION_MINOR >= 1
            !flag_isoc23
         #else
            !flag_isoc2x
         #endif
      ) {
         return dialect::c11;
      }
      return dialect::c23;
   }
}