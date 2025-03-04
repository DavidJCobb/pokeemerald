#pragma once
#include <gcc-plugin.h>
#include <c-family/c-pragma.h>

namespace pragma_handlers {
   extern void serialized_offset_to_constant(cpp_reader*);
}