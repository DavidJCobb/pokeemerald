#pragma once
#include <optional>
#include <utility> // std::pair
#include <vector>
#include "lu/singleton.h"
#include "codegen/serialization_item.h"

class last_generation_result;
class last_generation_result : public lu::singleton<last_generation_result> {
   public:
      using offset_size_pair   = std::pair<size_t, size_t>;
      using sector_offset_info = std::vector<offset_size_pair>;
      
      using item_list          = std::vector<codegen::serialization_item>;
      using sectored_item_list = std::vector<item_list>;
      
   protected:
      bool _empty = true;
      struct {
         sectored_item_list verbatim;
         sectored_item_list expanded;
      } _items_by_sector;
      std::vector<sector_offset_info> offsets_by_sector_expanded;
      
   public:
      constexpr bool empty() const noexcept { return this->_empty; }
      
      const item_list&           get_verbatim_sector_items(size_t sector_index) const noexcept;
      const item_list&           get_expanded_sector_items(size_t sector_index);
      const sector_offset_info&  get_expanded_sector_offsets(size_t sector_index);
      
      std::optional<size_t> find_containing_sector(const codegen::serialization_item&);
      std::optional<size_t> find_offset_within_sector(const codegen::serialization_item&); // in bits
      
      size_t sector_count() const noexcept { return this->_items_by_sector.verbatim.size(); }
      
      void update(
         const sectored_item_list& items_by_sector
      );
};