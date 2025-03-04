#pragma once
#include <cstdint>
#include <string>
#include "codegen/serialization_items/basic_segment.h"

namespace codegen::serialization_items {
   //
   // For serializing tagged unions: each union member (and the data therein) will be 
   // conditional on the union tag value. Multiple AND-linked conditions may be present 
   // (when dealing with nested unions).
   //
   struct condition_type {
      bool operator==(const condition_type&) const noexcept = default;
      
      std::vector<basic_segment> lhs;
      intmax_t rhs = 0;
      bool     is_else = false;
      
      std::string to_string() const;
   };
}