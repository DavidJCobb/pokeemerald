#pragma once
#include <string>
#include <variant>
#include <vector>
#include <gcc-plugin.h>
#include <c-family/c-pragma.h>
#include "gcc_wrappers/identifier.h"

namespace codegen {
   class generation_request {
      public:
         struct identifier {
            gcc_wrappers::identifier id;
            location_t loc = UNKNOWN_LOCATION;
            size_t dereference_count = 0;
            
            struct {
               gcc_wrappers::optional_node referent; // thing to which the identifier refers
            } cached;
         };
      
      public:
         struct {
            std::string read;
            std::string save;
         } function_names;
         std::vector<std::vector<identifier>> identifier_groups;
         struct {
            bool enable_debug_output = false;
         } settings;
         
         // location at which our data starts
         location_t start_location = UNKNOWN_LOCATION;
         
      public:
         bool from_pragma_parser(cpp_reader&);
         bool are_identifiers_valid(bool complain = true);
         bool are_function_names_valid(bool complain = true) const;
   };
}