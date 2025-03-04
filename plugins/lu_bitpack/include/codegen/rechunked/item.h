#pragma once
#include <memory>
#include <vector>
#include "codegen/rechunked/chunks/base.h"

namespace codegen {
   class serialization_item;
}

namespace codegen::rechunked {
   class item {
      public:
         item() {}
         item(const serialization_item&);
      
      public:
         std::vector<std::unique_ptr<chunks::base>> chunks;
         
         bool is_omitted_and_defaulted() const;
   };
}