#include <iostream>
#include <string>
#include "lu/stringf.h"
#include "codegen/debugging/print_sectored_serialization_items.h"
#include "codegen/serialization_item_list_ops/get_offsets_and_sizes.h"

namespace codegen::debugging {
   extern void print_sectored_serialization_items(const std::vector<std::vector<serialization_item>>& sectors) {
      for(size_t i = 0; i < sectors.size(); ++i) {
         const auto& sector = sectors[i];
         std::cerr << "Sector " << i << ":\n";
         
         auto offset_info = serialization_item_list_ops::get_offsets_and_sizes(sectors[i]);
         assert(offset_info.size() == sector.size());
         for(size_t j = 0; j < sector.size(); ++j) {
            auto& item = sector[j];
            auto& info = offset_info[j];
            
            std::cerr << " - ";
            if (!item.is_omitted) {
               std::cerr << lu::stringf("{%05u:%05u} ", info.first, info.second);
            }
            if (item.is_defaulted)
               std::cerr << "<D> ";
            if (item.is_omitted)
               std::cerr << "<O> ";
            std::cerr << item.to_string();
            std::cerr << '\n';
         }
      }
   }
}