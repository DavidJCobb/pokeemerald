#pragma once
#include <vector>
#include "codegen/serialization_item.h"

namespace codegen::serialization_item_list_ops {
   //
   // If there's a set of multiple serialization items in a row that refer 
   // to sequential array elements, fold them into a single serialization 
   // item that refers to an array slice.
   //
   extern void fold_sequential_array_elements(std::vector<serialization_item>&);
}