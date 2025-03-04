#include "codegen/serialization_item_list_ops/force_expand_but_values_only.h"

namespace codegen::serialization_item_list_ops {
   extern std::vector<serialization_item> force_expand_but_values_only(const std::vector<serialization_item>& list) {
      std::vector<serialization_item> out;
      for(auto& item : list) {
         if (item.is_padding())
            continue;
         if (item.is_omitted)
            continue;
         if (!item.can_expand()) {
            out.push_back(item);
            continue;
         }
         const auto expanded = item.expanded();
         for(auto& v : expanded) {
            out.push_back(v);
         }
      }
      return out;
   }
}