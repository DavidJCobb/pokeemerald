#include "codegen/serialization_item_list_ops/force_expand_unions_and_anonymous.h"
#include "lu/vectors/replace_item_with_vector.h"
#include "codegen/decl_descriptor.h"

namespace codegen::serialization_item_list_ops {
   extern void force_expand_unions_and_anonymous(std::vector<serialization_item>& list) {
      auto size = list.size();
      
      bool any_changes_made;
      do {
         any_changes_made = false;
         for(size_t i = 0; i < size; ++i) {
            auto& item = list[i];
            if (item.is_padding())
               continue;
            if (item.is_opaque_buffer())
               continue;
            
            assert(!item.segments.empty());
            assert(item.segments.back().is_basic());
            
            auto& back = item.segments.back().as_basic();
            auto& desc = item.descriptor();
            auto  type = *desc.types.serialized;
            if (!type.is_union()) {
               if (!type.is_record())
                  continue;
               if (!type.name().empty())
                  continue;
            }
            //
            // This is either a union, an anonymous struct, or an array of either.
            //
            any_changes_made = true;
            //
            // If it's an array, then expand it to the innermost slice.
            //
            while (back.array_accesses.size() < desc.array.extents.size()) {
               size_t rank   = back.array_accesses.size();
               size_t extent = desc.array.extents[rank];
               auto&  access = back.array_accesses.emplace_back();
               access.start = 0;
               access.count = extent;
            }
            //
            // Now expand the item.
            //
            const auto expanded = item.expanded();
            lu::vectors::replace_item_with_vector(list, i, expanded);
            size = list.size();
            --i;
         }
      } while (any_changes_made);
   }
}