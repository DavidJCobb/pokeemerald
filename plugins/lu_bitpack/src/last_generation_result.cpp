#include "last_generation_result.h"
#include <cassert>
#include "codegen/serialization_item_list_ops/force_expand_but_values_only.h"
#include "codegen/serialization_item_list_ops/get_offsets_and_sizes.h"
#include "codegen/decl_descriptor.h"
namespace serialization_item_list_ops {
   using namespace codegen::serialization_item_list_ops;
}
namespace {
   using item_list          = last_generation_result::item_list;
   using sectored_item_list = last_generation_result::sectored_item_list;
   using sector_offset_info = last_generation_result::sector_offset_info;
   using serialization_item = codegen::serialization_item;
}

const item_list& last_generation_result::get_verbatim_sector_items(size_t sector_index) const noexcept {
   assert(sector_index < this->sector_count());
   return this->_items_by_sector.verbatim[sector_index];
}
const item_list& last_generation_result::get_expanded_sector_items(size_t sector_index) {
   assert(sector_index < this->sector_count());
   auto& items_src   = this->_items_by_sector.verbatim[sector_index];
   auto& sectors_dst = this->_items_by_sector.expanded;
   if (sector_index >= sectors_dst.size()) {
      sectors_dst.resize(sector_index + 1);
      sectors_dst[sector_index] = serialization_item_list_ops::force_expand_but_values_only(items_src);
   }
   return sectors_dst[sector_index];
}
const sector_offset_info& last_generation_result::get_expanded_sector_offsets(size_t index) {
   assert(index < this->sector_count());
   auto& items = this->get_expanded_sector_items(index);
   if (this->offsets_by_sector_expanded.size() > index) {
      auto& info = this->offsets_by_sector_expanded[index];
      if (!info.empty())
         return info;
      if (items.empty())
         return info;
   } else {
      this->offsets_by_sector_expanded.resize(index + 1);
   }
   auto& info = this->offsets_by_sector_expanded[index];
   info = serialization_item_list_ops::get_offsets_and_sizes(items);
   return info;
}

std::optional<size_t> last_generation_result::find_containing_sector(const serialization_item& requested_item) {
   size_t count = this->sector_count();
   for(size_t i = 0; i < count; ++i) {
      const auto& items = this->get_expanded_sector_items(i);
      for(const auto& item : items)
         if (item.is_whole_or_part(requested_item))
            return i;
   }
   return {};
}
std::optional<size_t> last_generation_result::find_offset_within_sector(const serialization_item& requested_item) {
   size_t count = this->sector_count();
   for(size_t i = 0; i < count; ++i) {
      const auto& items = this->get_expanded_sector_items(i);
      for(size_t j = 0; j < items.size(); ++j) {
         const auto& item = items[j];
         if (!item.is_whole_or_part(requested_item))
            continue;
         
         const auto& offsets = this->get_expanded_sector_offsets(i);
         const auto& data    = offsets[j];
         size_t additional = 0;
         {
            //
            // Consider the following scenario: we have `int foo[7]`, and we query 
            // the location of `foo[3]`. If the entirety of `foo` fit into a single 
            // sector, then the only serialization item to find is `foo`, and the 
            // offset of that serialization item will be the offset of `foo[0]`. To 
            // find the offset of `foo[3]`, we have to drill down into the array.
            //
            // In the scenario where we want the offset of `foo[3]`, but only have 
            // a serialization item for `foo`:
            //
            //  - segm_a    == `foo[3]`
            //  - segm_b    == `foo`
            //  - depth     == 1
            //  - min_depth == 0
            //
            auto&  segm_a      = requested_item.segments.back().as_basic();
            auto&  segm_b      = item.segments[requested_item.segments.size() - 1].as_basic();
            size_t depth       = segm_a.array_accesses.size();
            size_t min_depth   = segm_b.array_accesses.size();
            assert(depth >= min_depth);
            if (depth >= 0) {
               size_t single_size = segm_a.desc->serialized_type_size_in_bits();
               auto&  extents     = segm_a.desc->array.extents;
               for(size_t k = 0; k < depth; ++k) {
                  size_t find_start = segm_a.array_accesses[k].start;
                  size_t base_start = 0;
                  if (segm_b.array_accesses.size() > k)
                     base_start = segm_b.array_accesses[k].start;
                  
                  size_t diff = find_start - base_start;
                  if (diff > 0) {
                     for(size_t l = k + 1; l < extents.size(); ++l)
                        diff *= extents[l];
                     additional += diff * single_size;
                  }
               }
            }
         }
         
         return data.first + additional;
      }
   }
   return {};
}

void last_generation_result::update(
   const sectored_item_list& items_by_sector
) {
   this->_empty = false;
   this->_items_by_sector.verbatim = items_by_sector;
   this->_items_by_sector.expanded.clear();
   this->offsets_by_sector_expanded.clear();
}