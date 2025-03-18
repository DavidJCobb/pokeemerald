#include "attribute_handlers/bitpack_misc_annotation.h"
#include "attribute_handlers/helpers/bp_attr_context.h"
#include "attribute_handlers/generic_bitpacking_data_option.h"
#include "gcc_wrappers/constant/string.h"
namespace gw {
   using namespace gcc_wrappers;
}

namespace attribute_handlers {
   extern tree bitpack_misc_annotation(tree* node_ptr, tree name, tree args, int flags, bool* no_add_attrs) {
      auto result = generic_bitpacking_data_option(node_ptr, name, args, flags, no_add_attrs);
      if (*no_add_attrs) {
         return result;
      }
      
      helpers::bp_attr_context context(node_ptr, name, flags);
      
      gw::constant::optional_string data;
      {
         auto next = TREE_VALUE(args);
         if (next != NULL_TREE && gw::constant::string::raw_node_is(next)) {
            data = next;
         } else {
            context.report_error("argument must be a string constant");
         }
      }
      
      if (data) {
         if (data->value().empty()) {
            context.report_error("argument cannot be blank");
         }
      }
      
      if (context.has_any_errors()) {
         *no_add_attrs = true;
      }
      return NULL_TREE;
   }
}