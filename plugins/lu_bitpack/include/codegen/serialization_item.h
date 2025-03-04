#pragma once
#include <memory> // GCC headers fuck up string-related STL identifiers that <memory> depends on
#include <string>
#include <vector>
#include "gcc_wrappers/type/base.h"
#include "codegen/serialization_items/segment.h"
#include "codegen/array_access_info.h"

namespace bitpacking {
   class data_options;
}
namespace codegen {
   class decl_descriptor;
}

namespace codegen {
   class serialization_item {
      public:
         using basic_segment   = serialization_items::basic_segment;
         using padding_segment = serialization_items::padding_segment;
         using condition_type  = serialization_items::condition_type;
         using segment         = serialization_items::segment;
         
      public:
         bool is_defaulted : 1 = false;
         bool is_omitted   : 1 = false;
         //
         std::vector<segment> segments;
         
      public:
         void append_segment(const decl_descriptor&);
      
         const decl_descriptor& descriptor() const noexcept;
         const bitpacking::data_options& options() const noexcept;
         
         size_t size_in_bits() const;
         size_t single_size_in_bits() const; // ignore array slice access, but not the value type being an array
         
         // non-omitted; omitted and defaulted; or omitted and contains defaulteds
         bool affects_output_in_any_way() const;
         
         bool can_expand() const;
         
         bool is_opaque_buffer() const;
         bool is_padding() const;
         bool is_union() const;
         
         // Think of this as akin to a `String.starts_with` check.
         bool is_whole_or_part(const serialization_item&) const;
         
         // Checks whether A is B, contains all of B, or is contained 
         // by all of B.
         bool has_whole_overlap(const serialization_item&) const;
      
      protected:
         std::vector<serialization_item> _expand_record() const;
         std::vector<serialization_item> _expand_externally_tagged_union() const;
         std::vector<serialization_item> _expand_internally_tagged_union() const;
      
      public:
         std::vector<serialization_item> expanded() const;
         
         std::string to_string() const;
   };
}