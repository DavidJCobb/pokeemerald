#pragma once
#include "lu/strings/zview.h"
#include "gcc_wrappers/type/base.h"

namespace gcc_wrappers {
   class identifier;
}

namespace gcc_wrappers::type {
   extern optional_base lookup_by_name(lu::strings::zview);
   extern optional_base lookup_by_name(identifier);
}