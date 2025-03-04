#include "lu/strings/handle_kv_string.h"
#include <iostream>
#include <charconv> // std::from_chars
#include <stdexcept>
#include "lu/strings/trim.h"
#include "lu/stringf.h"

namespace lu::strings::_impl {
   //
   // Parse a string of a form like `key=value,key,key=value`, checking if the keys 
   // are present in a list of known parameters and if so, invoking the parameters' 
   // handlers as appropriate.
   //
   void handle_kv_string(std::string_view data, const kv_string_param* params, size_t param_count) {
      if (data.empty()) {
         return;
      }
      size_t i    = -1;
      size_t prev;
      do {
         std::string_view subarg;
         
         prev = i;
         i    = data.find_first_of(",=", prev + 1);
         if (i == std::string::npos) {
            subarg = data.substr(prev + 1);
         } else {
            subarg = data.substr(prev + 1, i - (prev + 1));
         }
         trim_in_place(subarg);
         for(size_t _pi = 0; _pi < param_count; ++_pi) {
            auto& p = params[_pi];
            
            if (subarg != p.name)
               continue;
            
            if (p.has_param) {
               if (i == std::string::npos || data[i] != '=') {
                  throw std::runtime_error(stringf(
                     "key is missing a value (seen: %s...)",
                     std::string(subarg).c_str()
                  ));
               }
               std::string_view value;
               
               size_t j = data.find_first_of(",=", i + 1);
               if (j == std::string::npos) {
                  value = data.substr(i + 1);
                  i = data.size();
                  trim_in_place(value);
               } else {
                  value = data.substr(i + 1, j - (i + 1));
                  i = j;
                  trim_in_place(value);
                  if (data[j] == '=') {
                     throw std::runtime_error(stringf(
                        "`key=value=value` isn't valid (seen: %s=%s=...)",
                        std::string(subarg).c_str(),
                        std::string(value).c_str()
                     ));
                  }
               }
               
               int value_int = 0;
               if (p.int_param) {
                  auto conv = std::from_chars(
                     value.data(),
                     value.data() + value.size(),
                     value_int
                  );
                  if (conv.ec != std::errc{}) {
                     throw std::runtime_error(stringf(
                        "value is not a number (seen: %s=%s)",
                        std::string(subarg).c_str(),
                        std::string(value).c_str()
                     ));
                  }
                  if (conv.ptr < value.data() + value.size()) {
                     throw std::runtime_error(stringf(
                        "unexpected non-whitespace content after number (seen: %s=%s)",
                        std::string(subarg).c_str(),
                        std::string(value).c_str()
                     ));
                  }
               }
               p.handler(value, value_int);
            } else {
               if (i != std::string::npos && data[i] == '=') {
                  throw std::runtime_error(stringf(
                     "unexpected value for a key which should not have one (seen: %s=...)",
                     std::string(subarg).c_str()
                  ));
               }
               p.handler("", 0);
            }
            break;
         }
      } while (i != std::string::npos && i + 1 < data.size());
   }
}