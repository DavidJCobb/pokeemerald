#include "lu/strings/trim.h"
#include <string> // std::string::npos

namespace lu::strings {
   extern std::string_view ltrim(const std::string_view& src) {
      auto dst = src;
      ltrim_in_place(dst);
      return dst;
   }
   extern std::string_view rtrim(const std::string_view& src) {
      auto dst = src;
      rtrim_in_place(dst);
      return dst;
   }
   extern std::string_view trim(const std::string_view& src) {
      auto dst = src;
      ltrim_in_place(dst);
      rtrim_in_place(dst);
      return dst;
   }
   
   extern void ltrim_in_place(std::string_view& view) {
      auto i = view.find_first_not_of(" \n\r\t");
      if (i == std::string::npos || i == 0)
         return;
      view.remove_prefix(i);
   }
   extern void rtrim_in_place(std::string_view& view) {
      auto i = view.find_last_not_of(" \n\r\t");
      if (i == std::string::npos)
         return;
      view = view.substr(0, i + 1);
   }
   extern void trim_in_place(std::string_view& view) {
      ltrim_in_place(view);
      rtrim_in_place(view);
   }
}