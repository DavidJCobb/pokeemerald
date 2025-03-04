#include "codegen/serialization_item_list_ops/divide_items_by_sectors.h"
#include <algorithm> // std::copy, std::min
#include <cassert>
#include "codegen/decl_descriptor.h"

#include "codegen/serialization_item_list_ops/fold_sequential_array_elements.h"
#include "codegen/serialization_item_list_ops/force_expand_unions_and_anonymous.h"
#include "codegen/serialization_item_list_ops/force_expand_omitted_and_defaulted.h"

namespace {
   using sector_type = std::vector<codegen::serialization_item>;
   using sector_list = std::vector<sector_type>;

   using segment_condition = codegen::serialization_items::condition_type;

   struct overall_state {
      size_t      bits_per_sector = 0;
      sector_list sectors;
   };

   //
   // Leftover from when we tried to implement sector splitting of unions. 
   // Retained in case we think of a workable solution in the future. (See 
   // Markdown-format documentation for problems.) Same basic design as 
   // with `serialization_item_list_ops::get_offsets_and_sizes`.
   //
   struct branch_state {
      overall_state*    overall;
      segment_condition condition;
      size_t bits_remaining = 0;
      size_t current_sector = 0;
      
      branch_state() {} // needed for std::vector
      branch_state(overall_state& o) : overall(&o), bits_remaining(o.bits_per_sector) {}
      
      void insert(const codegen::serialization_item& item) {
         if (overall->sectors.size() <= this->current_sector)
            overall->sectors.resize(this->current_sector + 1);
         overall->sectors[this->current_sector].push_back(item);
         
         if (!item.is_omitted)
            this->bits_remaining -= item.size_in_bits();
      }
      void next() {
         ++this->current_sector;
         this->bits_remaining = overall->bits_per_sector;
      }
      
      constexpr bool is_at_or_ahead_of(const branch_state& o) const noexcept {
         if (this->current_sector > o.current_sector)
            return true;
         if (this->current_sector < o.current_sector)
            return false;
         return this->bits_remaining <= o.bits_remaining;
      }
      
      void catch_up_to(const branch_state& o) {
         assert(o.is_at_or_ahead_of(*this));
         this->bits_remaining = o.bits_remaining;
         this->current_sector = o.current_sector;
      }
   };
}

namespace codegen::serialization_item_list_ops {
   extern std::vector<std::vector<serialization_item>> divide_items_by_sectors(
      size_t sector_size_in_bits,
      std::vector<serialization_item> src
   ) {
      overall_state overall;
      overall.bits_per_sector = sector_size_in_bits;
      assert(sector_size_in_bits > 0);
      
      branch_state root{overall};
      std::vector<branch_state> branches;
      
      size_t size = src.size();
      for(size_t i = 0; i < size; ++i) {
         const auto& item = src[i];
         assert(!item.segments.empty());
         
         //
         // Union expansion and conditions are not supported w.r.t. sector 
         // splitting. Unions must be expanded only as a post-process step, 
         // not in advance. By extension, padding also cannot be split.
         //
         assert(!item.is_padding());
         for(auto& segment : item.segments) {
            assert(!segment.condition.has_value());
         }
         
         auto& current_branch = branches.empty() ? root : branches.back();
         const size_t remaining = current_branch.bits_remaining;
         
         if (item.is_omitted) {
            //
            // Just because an item is omitted doesn't mean it won't affect the 
            // generated code. For example, an omitted-and-defaulted field needs 
            // to be set to its default value when reading from the bitstream; 
            // and an omitted struct can't be defaulted, but may contain members 
            // that are defaulted and so need the same treatment. We must retain  
            // serialization items for these omitted-and-defaulted values.
            //
            if (!item.affects_output_in_any_way())
               continue;
            current_branch.insert(item);
            continue;
         }
         
         size_t bitcount = item.size_in_bits();
         if (bitcount <= remaining) {
            //
            // If the entire item can fit, then insert it unmodified.
            //
            current_branch.insert(item);
            continue;
         }
         //
         // Else, the item can't fit.
         //
         if (remaining > 0) {
            //
            // The item can't fit. Expand it if possible; if not, push it to 
            // the next sector (after we verify that it'll even fit there).
            //
            // NOTE: Splitting unions across sector boundaries involves some 
            // very, very complicated logistics, so we refuse to do it.
            //
            if (item.can_expand() && !item.is_union()) {
               auto   expanded       = item.expanded();
               size_t expanded_count = expanded.size();
               expanded.resize(size - i - 1 + expanded_count);
               std::copy(
                  src.begin() + i + 1,
                  src.end(),
                  expanded.begin() + expanded_count
               );
               src  = expanded;
               size = expanded.size();
               i    = -1;
               continue;
            }
            //
            // Cannot expand the item.
            //
            if (bitcount > sector_size_in_bits) {
               throw std::runtime_error(std::string("found an element that is too large to fit in any sector: ") + item.to_string());
            }
         }
         
         current_branch.next();
         --i; // re-process current item
         continue;
      }
      
      //
      // POST-PROCESS: CONSECUTIVE ARRAY ELEMENTS TO ARRAY SLICES
      //
      // Join exploded arrays into array slices wherever possible. For example, 
      // { `foo[0]`, `foo[1]`, `foo[2]` } should join to `foo[0:3]` or, if the 
      // entire array is represented there, just `foo`. This will help us with 
      // generating `for` loops to handle these items.
      //
      for(auto& sector : overall.sectors) {
         serialization_item_list_ops::fold_sequential_array_elements(sector);
      }
      
      //
      // POST-PROCESS: FORCE-EXPAND UNIONS, ANONYMOUS STRUCTS, AND ARRAYS THEREOF
      //
      // We can't split unions, but we want them expanded after splitting is 
      // done.
      //
      for(auto& sector : overall.sectors) {
         serialization_item_list_ops::force_expand_unions_and_anonymous(sector);
      }
      
      //
      // POST-PROCESS: FORCE-EXPAND OMITTED-AND_DEFAULTED
      //
      // This will ensure that we properly generate instructions to set default 
      // values. It's important primarily for omitted instances of named structs 
      // that contain defaulted members.
      //
      for(auto& sector : overall.sectors) {
         serialization_item_list_ops::force_expand_omitted_and_defaulted(sector);
      }
      
      return overall.sectors;
   }
}