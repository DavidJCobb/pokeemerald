#pragma once
#include <vector>
#include "codegen/serialization_item.h"

namespace codegen::serialization_item_list_ops {
   extern std::vector<serialization_item> force_expand_but_values_only(const std::vector<serialization_item>&);
}