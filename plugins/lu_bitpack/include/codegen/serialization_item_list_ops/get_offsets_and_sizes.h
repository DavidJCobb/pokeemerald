#pragma once
#include <utility>
#include <vector>
#include "codegen/serialization_item.h"

namespace codegen::serialization_item_list_ops {
   //
   // Return the offset and size of each serialization item, measured in bits, 
   // accounting for their conditions.
   //
   extern std::vector<std::pair<size_t, size_t>> get_offsets_and_sizes(const std::vector<serialization_item>&);
}