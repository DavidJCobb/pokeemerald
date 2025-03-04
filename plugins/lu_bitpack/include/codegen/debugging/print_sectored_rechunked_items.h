#pragma once
#include <vector>
#include "codegen/rechunked/item.h"

namespace codegen::debugging {
   extern void print_sectored_rechunked_items(const std::vector<std::vector<rechunked::item>>&);
}