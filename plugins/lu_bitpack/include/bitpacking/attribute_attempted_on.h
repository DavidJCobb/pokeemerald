#pragma once
#include "lu/strings/zview.h"

namespace gcc_wrappers {
   class attribute_list;
}

namespace bitpacking {
   extern bool attribute_attempted_on(gcc_wrappers::attribute_list, lu::strings::zview attr_name);
}