#pragma once
#include <vector>
#include "codegen/serialization_item.h"

namespace codegen::serialization_item_list_ops {
   //
   // Forcibly expand serialization items that represent unions, anonymous 
   // structs, or arrays thereof.
   //
   extern void force_expand_unions_and_anonymous(std::vector<serialization_item>&);
}