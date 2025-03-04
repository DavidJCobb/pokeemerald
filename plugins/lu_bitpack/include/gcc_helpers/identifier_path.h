#pragma once
#include <string>
#include <vector>
#include "gcc_wrappers/node.h"
#include "gcc_wrappers/optional.h"
#include "gcc_wrappers/identifier.h"
#include <c-family/c-pragma.h>

namespace gcc_helpers {
   class identifier_path {
      public:
         struct segment {
            cpp_ttype token = (cpp_ttype)-1;
            gcc_wrappers::identifier id;
         };
         std::vector<segment> path;
         
      protected:
         std::string _fully_qualified_name(size_t up_to) const;
      
      public:
         void parse(cpp_reader&); // throws pragma_parse_exception
         gcc_wrappers::optional_node resolve() const; // throws std::runtime_error
         
         std::string fully_qualified_name() const;
   };
};