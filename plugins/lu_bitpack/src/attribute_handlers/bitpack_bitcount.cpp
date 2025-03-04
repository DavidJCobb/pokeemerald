#include "attribute_handlers/bitpack_bitcount.h"
#include <cinttypes> // PRIdMAX and friends
#include "attribute_handlers/helpers/bp_attr_context.h"
#include "attribute_handlers/generic_bitpacking_data_option.h"
#include "gcc_wrappers/constant/integer.h"
#include <diagnostic.h>
namespace gw {
   using namespace gcc_wrappers;
}

namespace attribute_handlers {
   extern tree bitpack_bitcount(tree* node_ptr, tree name, tree args, int flags, bool* no_add_attrs) {
      auto result = generic_bitpacking_data_option(node_ptr, name, args, flags, no_add_attrs);
      if (*no_add_attrs) {
         return result;
      }
      
      helpers::bp_attr_context context(node_ptr, name, flags);
      context.check_and_report_applied_to_integral();
      context.check_and_report_contradictory_x_options(helpers::x_option_type::integral);
      
      std::optional<intmax_t> v;
      {
         auto next = TREE_VALUE(args);
         if (next != NULL_TREE && gw::constant::integer::raw_node_is(next)) {
            v = gw::constant::integer::wrap(next).value<intmax_t>();
         }
      }
      if (!v.has_value()) {
         context.report_error("argument must be an integer constant");
      } else {
         auto bc = *v;
         if (bc <= 0) {
            context.report_error("argument cannot be zero or negative (seen: %<%" PRIdMAX "%>)", bc);
         } else if (bc > (intmax_t)sizeof(size_t) * 8) {
            warning(OPT_Wattributes, "%qE attribute specifies an unusually large bitcount; is this intentional? (seen: %<%" PRIdMAX "%>)", name, bc);
         }
      }
      
      if (context.has_any_errors()) {
         *no_add_attrs = true;
      }
      return NULL_TREE;
   }
}