#pragma once
#include <memory>
#include "xmlgen/xml_element.h"

namespace codegen::stats {
   class sector {
      public:
         size_t total_packed_size = 0;
         
         std::unique_ptr<xmlgen::xml_element> to_xml() const;
   };
}