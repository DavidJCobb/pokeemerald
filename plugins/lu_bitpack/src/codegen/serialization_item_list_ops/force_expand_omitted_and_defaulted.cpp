#include "codegen/serialization_item_list_ops/force_expand_omitted_and_defaulted.h"
#include "lu/vectors/replace_item_with_vector.h"
#include "codegen/decl_descriptor.h"

namespace codegen::serialization_item_list_ops {
   extern void force_expand_omitted_and_defaulted(std::vector<serialization_item>& list) {
      auto size = list.size();
      
      bool any_changes_made;
      do {
         any_changes_made = false;
         for(size_t i = 0; i < size; ++i) {
            auto& item = list[i];
            if (!item.is_omitted)
               continue;
            if (item.is_padding())
               continue;
            if (!item.can_expand())
               continue;
            
            assert(!item.segments.empty());
            assert(item.segments.back().is_basic());
            
            auto& desc = item.descriptor();
            if (!desc.is_or_contains_defaulted())
               continue;
            
            // Don't force-expand a wholly-omitted array.
            if (!desc.array.extents.empty())
               continue;
            
            const auto expanded = item.expanded();
            lu::vectors::replace_item_with_vector(list, i, expanded);
            size = list.size();
            --i;
         }
      } while (any_changes_made);
   }
}