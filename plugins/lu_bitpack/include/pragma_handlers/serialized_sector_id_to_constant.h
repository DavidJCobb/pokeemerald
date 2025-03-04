#pragma once
#include <gcc-plugin.h>
#include <c-family/c-pragma.h>

namespace pragma_handlers {
   extern void serialized_sector_id_to_constant(cpp_reader*);
}