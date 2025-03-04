#include "pragma_handlers/set_options.h"
#include "basic_global_state.h"
#include <diagnostic.h>

namespace pragma_handlers {
   extern void enable(cpp_reader* reader) {
      auto& bgs = basic_global_state::get();
      bgs.enabled = true;
      inform(UNKNOWN_LOCATION, "lu-bitpack: code generation enabled");
      {
         const auto& path = bgs.xml_output_path;
         if (!path.empty()) {
            if (path.ends_with(".xml")) {
               inform(UNKNOWN_LOCATION, "lu-bitpack: using XML output path set on the command line: %<%s%>", path.c_str());
            } else {
               warning(OPT_Wpragmas, "lu-bitpack: ignoring invalid XML output path set on the command line (must end in %<.xml%>): %<%s%>", path.c_str());
            }
         }
      }
   }
}