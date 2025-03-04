#include "pragma_handlers/set_options.h"
#include "gcc_wrappers/builtin_types.h"
#include "basic_global_state.h"
#include "pragma_parse_exception.h"
#include "bitpacking/requested_global_options.h"
#include <diagnostic.h>

namespace pragma_handlers {
   extern void set_options(cpp_reader* reader) {
      constexpr const char* this_pragma_name = "#pragma lu_bitpack set_options";
      
      // ensure we can use `get_fast` on the built-in types:
      gcc_wrappers::builtin_types::get();
      
      auto& state = basic_global_state::get();
      if (state.global_options_seen) {
         error("%qs: global options have already been set in this translation unit and cannot be redefined", this_pragma_name);
         return;
      }
      
      state.global_options_seen = true;
      try {
         auto requested = bitpacking::requested_global_options(*reader);
         state.global_options.grab(requested);
      } catch (const pragma_parse_exception& ex) {
         error_at(ex.location, ex.what());
         return;
      }
   }
}