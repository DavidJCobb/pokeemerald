#pragma once
#include <memory>
#include "codegen/stats/serializable.h"
#include "xmlgen/xml_element.h"
#include "gcc_wrappers/type/base.h"

namespace codegen::stats {
   class c_type : public serializable {
      public:
         c_type(gcc_wrappers::type::base);
         c_type(const c_type&) = default;
      
      public:
         gcc_wrappers::type::base type;
         struct {
            size_t align_of = 0;
            size_t size_of  = 0;
         } c_info;
         
         bool empty() const noexcept = delete;
         std::unique_ptr<xmlgen::xml_element> to_xml() const;
   };
}