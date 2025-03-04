#pragma once
#include <memory>
#include "codegen/instructions/base.h"
#include "codegen/func_pair.h"

namespace codegen {
   struct whole_struct_function_info {
      optional_func_pair functions;
      std::unique_ptr<instructions::base> instructions_root;
   };
}