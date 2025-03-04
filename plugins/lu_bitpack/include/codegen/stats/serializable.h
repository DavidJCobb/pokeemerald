#pragma once
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include "xmlgen/xml_element.h"

namespace codegen::stats {
   class serializable {
      public:
         struct count_in_sector {
            size_t sector_index = 0;
            size_t count        = 0;
         };
         struct count_in_top_level_identifier {
            std::string identifier;
            size_t      count;
         };
      
      public:
         struct {
            size_t total_packed   = 0;
            size_t total_unpacked = 0;
         } bitcounts;
         struct {
            size_t total = 0;
            std::vector<count_in_sector> by_sector;
            std::vector<count_in_top_level_identifier> by_top_level_identifier;
         } counts;
         
      public:
         bool empty() const;
         std::unique_ptr<xmlgen::xml_element> to_xml() const;
         
      public:
         void add_count_in_sector(size_t sector_index, size_t count);
         void add_count_in_top_level_identifier(std::string_view identifier, size_t count);
   };
}