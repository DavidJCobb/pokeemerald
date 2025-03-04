#pragma once
#include <stdexcept>
#include <string>
#include "lu/stringf.h"
// GCC:
#include <gcc-plugin.h>
#include <tree.h>

class pragma_parse_exception : public std::runtime_error {
   public:
      pragma_parse_exception(location_t loc, const std::string& what);
      
      template<typename... Args>
      pragma_parse_exception(location_t loc, const char* fmt, Args&&... args)
      :
         std::runtime_error(lu::stringf(fmt, args...)),
         location(loc)
      {}
      
   public:
      const location_t location = UNKNOWN_LOCATION;
};