#pragma once
#include <string_view>

namespace lu::strings {
   extern std::string_view ltrim(const std::string_view&);
   extern std::string_view rtrim(const std::string_view&);
   extern std::string_view trim(const std::string_view&);
   
   extern void ltrim_in_place(std::string_view&);
   extern void rtrim_in_place(std::string_view&);
   extern void trim_in_place(std::string_view&);
}