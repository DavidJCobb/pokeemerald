#include "xmlgen/string_as_cdata.h"

namespace xmlgen {
   extern std::string string_as_cdata(std::string_view src) {
      constexpr const std::string_view token_start       = "<![CDATA[";
      constexpr const std::string_view token_end         = "]]>";
      constexpr const std::string_view escaped_token_end = "]]&gt;";
      
      
      size_t i = src.find(token_end);
      if (i == std::string::npos) {
         std::string out;
         out.reserve(src.size() + token_start.size() + token_end.size());
         out += token_start;
         out += src;
         out += token_end;
         return out;
      }
   
      std::string out;
      out += token_start;
      out += src.substr(0, i);
      out += token_end;
      out += escaped_token_end;
      
      size_t prev;
      while (i < src.size()) {
         out += token_start;
         
         prev = i + token_end.size();
         i    = src.find(token_end, prev);
         if (i != std::string::npos) {
            out += src.substr(prev, i - prev);
            out += token_end;
            out += escaped_token_end;
         } else {
            out += src.substr(prev);
            out += token_end;
         }
      }
      return out;
   }
}