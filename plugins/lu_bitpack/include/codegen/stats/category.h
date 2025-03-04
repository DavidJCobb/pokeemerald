#pragma once
#include <memory>
#include <string_view>
#include "codegen/stats/serializable.h"
#include "xmlgen/xml_element.h"

namespace codegen::stats {
   class category : public serializable {
      public:
         bool empty() const noexcept = delete;
         std::unique_ptr<xmlgen::xml_element> to_xml(std::string_view category_name) const;
   };
}