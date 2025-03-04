#pragma once
#include <gcc-plugin.h>
#include <c-family/c-pragma.h>

namespace pragma_handlers {
   extern void set_options(cpp_reader*);
}