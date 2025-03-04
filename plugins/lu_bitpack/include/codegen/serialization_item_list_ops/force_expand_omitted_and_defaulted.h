#pragma once
#include <vector>
#include "codegen/serialization_item.h"

namespace codegen::serialization_item_list_ops {
   extern void force_expand_omitted_and_defaulted(std::vector<serialization_item>&);
}