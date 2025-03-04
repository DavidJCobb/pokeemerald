#pragma once
#include "codegen/serialization_item.h"

namespace codegen::serialization_item_list_ops {
   //
   // Similar to `get_offsets_and_sizes`, but avoids creating a vector 
   // and whatnot.
   //
   extern size_t get_total_serialized_size(const std::vector<serialization_item>&);
}