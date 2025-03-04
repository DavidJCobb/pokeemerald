#pragma once
#include <string>
#include <string_view>

namespace xmlgen {
   extern void write_string_as_text_content(std::string_view src, std::string& dst);
}