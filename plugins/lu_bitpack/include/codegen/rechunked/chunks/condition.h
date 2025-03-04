#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include "codegen/rechunked/chunks/base.h"

namespace codegen::serialization_items {
   class basic_segment;
   class condition;
}

namespace codegen::rechunked::chunks {
   class condition : public base {
      public:
         static constexpr const type chunk_type = type::condition;
         virtual type get_type() const noexcept override { return chunk_type; }
         
         virtual bool compare(const base&) const noexcept override;
      
      public:
         struct _ {
            // Can contain any chunks except for `condition` and `padding` chunks.
            std::vector<std::unique_ptr<base>> chunks;
         } lhs;
         intmax_t rhs = 0;
         bool     is_else = false;
         
      public:
         void set_lhs_from_segments(const std::vector<serialization_items::basic_segment>&);
         
         bool compare_lhs(const condition& other) const noexcept;
   };
}