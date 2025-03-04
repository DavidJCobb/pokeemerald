#include "xmlgen/write_string_as_text_content.h"

namespace xmlgen {
   extern void write_string_as_text_content(std::string_view src, std::string& dst) {
      constexpr const char* characters_of_interest = "<>&]";
      constexpr const char* cdata_terminator       = "]]>";
      
      size_t i = src.find_first_of(characters_of_interest);
      if (i == std::string::npos) {
         dst += src;
         return;
      }
      size_t first = i;
      
      bool requires_escape = false;
      while (i != std::string::npos) {
         if (src[i] != ']') {
            requires_escape = true;
            break;
         }
         if (src.substr(i).starts_with(cdata_terminator)) {
            requires_escape = true;
            break;
         }
         i = src.find_first_of(characters_of_interest, i + 1);
      }
      if (!requires_escape) {
         dst += src;
         return;
      }
      
      i = first;
      size_t prev = 0;
      while (i != std::string::npos) {
         dst += src.substr(prev, i - prev);
         switch (src[i]) {
            case '<':
               dst += "&lt;";
               break;
            case '>':
               dst += "&gt;";
               break;
            case '&':
               dst += "&amp;";
               break;
            case ']':
               if (src.substr(i).starts_with(cdata_terminator)) {
                  dst += "]]&gt;";
                  prev = i + 3;
                  i    = src.find_first_of(characters_of_interest, prev);
                  continue;
               }
               dst += src[i];
               break;
         }
         prev = i + 1;
         i    = src.find_first_of(characters_of_interest, prev);
      }
      dst += src.substr(prev);
   }
}