#pragma once
#include <string>
#include <string_view>

namespace xmlgen {
   extern void write_string_as_attribute_value(std::string_view src, char delim, std::string& dst);
}