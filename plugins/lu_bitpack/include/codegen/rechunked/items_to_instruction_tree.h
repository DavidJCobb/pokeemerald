#pragma once
#include <memory>
#include <vector>
#include "codegen/instructions/base.h"
#include "codegen/rechunked/item.h"

namespace codegen::rechunked {
   extern std::unique_ptr<instructions::container> items_to_instruction_tree(
      const std::vector<item>&
   );
}