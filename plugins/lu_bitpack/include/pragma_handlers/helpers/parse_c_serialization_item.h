#pragma once
#include <gcc-plugin.h>
#include <c-family/c-pragma.h>
#include "codegen/serialization_item.h"

namespace pragma_handlers::helpers {
   // May throw pragma_parse_exception.
   extern codegen::serialization_item parse_c_serialization_item(cpp_reader*);
}