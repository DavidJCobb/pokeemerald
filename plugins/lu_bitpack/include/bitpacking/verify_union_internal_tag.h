#pragma once
#include "lu/strings/zview.h"
#include "gcc_wrappers/type/untagged_union.h"

namespace bitpacking {
   // If any errors, reports them and returns false.
   extern bool verify_union_internal_tag(
      gcc_wrappers::type::untagged_union type,
      lu::strings::zview                 tag_identifier,
      bool silent = false
   );
}