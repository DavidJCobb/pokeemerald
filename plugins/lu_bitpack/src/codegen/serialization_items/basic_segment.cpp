#include "codegen/serialization_items/basic_segment.h"
#include "lu/stringf.h"
#include "bitpacking/data_options.h"
#include "codegen/decl_descriptor.h"

namespace codegen::serialization_items {
   const bitpacking::data_options& basic_segment::options() const noexcept {
      assert(this->desc != nullptr);
      return this->desc->options;
   }
   
   size_t basic_segment::size_in_bits() const noexcept {
      auto size = this->single_size_in_bits();
      if (!this->array_accesses.empty()) {
         size *= this->array_accesses.back().count;
      }
      return size;
   }
   size_t basic_segment::single_size_in_bits() const noexcept {
      assert(this->desc != nullptr);
      auto  size    = this->desc->serialized_type_size_in_bits();
      auto& extents = this->desc->array.extents;
      for(size_t i = this->array_accesses.size(); i < extents.size(); ++i) {
         size *= extents[i];
      }
      return size;
   }
   
   bool basic_segment::is_array() const {
      assert(this->desc != nullptr);
      auto& desc = *this->desc;
      
      if (this->array_accesses.size() < desc.array.extents.size())
         return true;
      
      return false;
   }
   size_t basic_segment::array_extent() const {
      assert(this->desc != nullptr);
      auto& extents  = this->desc->array.extents;
      auto& accesses = this->array_accesses;
      if (accesses.size() < extents.size()) {
         return extents[accesses.size()];
      }
      return 1;
   }
   
   std::string basic_segment::to_string() const {
      std::string out;
      
      size_t dereference_count = this->desc->variable.dereference_count;
      if (dereference_count) {
         out += '(';
         for(size_t i = 0; i < dereference_count; ++i)
            out += '*';
      }
      out += this->desc->decl.name();
      if (dereference_count) {
         out += ')';
      }
      for(auto& access : this->array_accesses) {
         out += '[';
         if (access.count == 1) {
            out += lu::stringf("%u", (int)access.start);
         } else {
            out += lu::stringf("%u:%u", (int)access.start, (int)(access.start + access.count));
         }
         out += ']';
      }
      
      return out;
   }
   
}