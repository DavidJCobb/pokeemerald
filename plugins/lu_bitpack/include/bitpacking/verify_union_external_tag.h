#pragma once
#include "lu/strings/zview.h"
#include "gcc_wrappers/decl/field.h"

namespace bitpacking {
   // If any errors, reports them and returns false.
   extern bool verify_union_external_tag(
      gcc_wrappers::decl::field union_decl,
      lu::strings::zview        tag_identifier,
      bool silent = false
   );
}