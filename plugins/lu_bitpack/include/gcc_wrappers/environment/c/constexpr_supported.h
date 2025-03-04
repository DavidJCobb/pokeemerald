#pragma once
#include "gcc_headers/plugin-version.h"

namespace gcc_wrappers::environment::c {
   // Indicates whether the version of GCC we've been built for supports 
   // `constexpr` in C. DOES NOT indicate whether we're currently being 
   // used to compile C23 code; check `current_dialect()`, here in this 
   // namespace, for that.
   constexpr const bool constexpr_supported = (GCCPLUGIN_VERSION_MAJOR >= 13 && GCCPLUGIN_VERSION_MINOR >= 1);
}