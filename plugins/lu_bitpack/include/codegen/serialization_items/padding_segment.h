#pragma once

namespace codegen::serialization_items {
   class padding_segment {
      public:
         bool operator==(const padding_segment&) const noexcept = default;
         
      public:
         size_t bitcount = 0;
   };
}