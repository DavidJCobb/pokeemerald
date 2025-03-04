#include "xmlgen/write_string_as_attribute_value.h"

namespace xmlgen {
   extern void write_string_as_attribute_value(std::string_view src, char delim, std::string& dst) {
      size_t from = 0;
      size_t i    = 0;
      do {
         i = src.find_first_of("<>'\"&\x09\x0A\x0D", from);
         if (i == std::string::npos) {
            dst += src.substr(from);
            break;
         }
         dst += src.substr(from, i - from);
         switch (src[i]) {
            case 0x09:
            case 0x0A:
            case 0x0D:
               dst += ' ';
               break;
            case '<':
               dst += "&lt;";
               break;
            case '>':
               dst += "&gt;";
               break;
            case '\'':
               if (src[i] == delim) {
                  dst += "&apos;";
               } else {
                  dst += '\'';
               }
               break;
            case '"':
               if (src[i] == delim) {
                  dst += "&quot;";
               } else {
                  dst += '"';
               }
               break;
            case '&':
               dst += "&amp;";
               break;
         }
         from = i + 1;
      } while (true);
   }
}