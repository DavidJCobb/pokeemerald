#pragma once
#include <vector>
#include "codegen/serialization_item.h"

namespace codegen::debugging {
   extern void print_sectored_serialization_items(const std::vector<std::vector<serialization_item>>&);
}