#include "xmlgen/is_valid_name.h"

static bool _is_name_start_char(char c) {
   if (c == ':' || c == '_')
      return true;
   if (c >= 'A' && c <= 'Z')
      return true;
   if (c >= 'a' && c <= 'z')
      return true;
   if (c >= 0xC0 && c <= 0xD6)
      return true;
   if (c >= 0xD8 && c <= 0xF6)
      return true;
   if (c >= 0xF8 && c <= 0xFF)
      return true;
   
   if constexpr (sizeof(c) > 1) {
      if (c >= 0xF8 && c <= 0x2FF)
         return true;
      if (c >= 0x370 && c <= 0x37D)
         return true;
      if (c >= 0x37F && c <= 0x1FFF)
         return true;
      if (c == 0x200C || c == 0x200D)
         return true;
      if (c >= 0x2070 && c <= 0x218F)
         return true;
      if (c >= 0x2C00 && c <= 0x2FEF)
         return true;
      if (c >= 0x3001 && c <= 0xD7FF)
         return true;
      if (c >= 0xF900 && c <= 0xFDCF)
         return true;
      if (c >= 0xFDF0 && c <= 0xFFFD)
         return true;
      if constexpr (sizeof(c) > 2) {
         if (c >= 0x10000 && c <= 0xEFFFF)
            return true;
      }
   }
   
   return false;
}

static bool _is_name_char(char c) {
   if (_is_name_start_char(c))
      return true;
   if (c == '-' || c == '.')
      return true;
   if (c >= '0' && c <= '9')
      return true;
   if (c == 0xB7)
      return true;
   if constexpr (sizeof(c) > 1) {
      if (c >= 0x300 && c <= 0x36F)
         return true;
      if (c == 0x203F || c == 0x2040)
         return true;
   }
   return false;
}

namespace xmlgen {
   extern bool is_valid_name(std::string_view data) {
      if (data.empty())
         return false;
      
      if (!_is_name_start_char(data[0]))
         return false;
      
      for(size_t i = 1; i < data.size(); ++i)
         if (!_is_name_char(data[i]))
            return false;
      
      return true;
   }
}