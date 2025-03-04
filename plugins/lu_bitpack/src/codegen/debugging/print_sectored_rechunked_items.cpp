#include <iostream>
#include "codegen/debugging/print_sectored_rechunked_items.h"
#include "codegen/rechunked/item_to_string.h"

namespace codegen::debugging {
   extern void print_sectored_rechunked_items(const std::vector<std::vector<rechunked::item>>& sectors) {
      for(size_t i = 0; i < sectors.size(); ++i) {
         const auto& sector = sectors[i];
         std::cerr << "Sector " << i << ":\n";
         for(auto& item : sector) {
            std::cerr << " - ";
            if (item.is_omitted_and_defaulted()) {
               std::cerr << "omitted and defaulted: ";
            }
            std::cerr << rechunked::item_to_string(item);
            std::cerr << '\n';
         }
      }
   }
}