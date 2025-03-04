#include "attribute_handlers/bitpack_string.h"
#include "attribute_handlers/helpers/bp_attr_context.h"
#include "attribute_handlers/generic_bitpacking_data_option.h"
#include "gcc_wrappers/type/array.h"
#include "gcc_wrappers/attribute_list.h"
namespace gw {
   using namespace gcc_wrappers;
}

namespace attribute_handlers {
   extern tree bitpack_string(tree* node_ptr, tree name, tree args, int flags, bool* no_add_attrs) {
      auto result = generic_bitpacking_data_option(node_ptr, name, args, flags, no_add_attrs);
      if (*no_add_attrs) {
         return result;
      }
      
      helpers::bp_attr_context context(node_ptr, name, flags);
      context.check_and_report_contradictory_x_options(helpers::x_option_type::string);
      
      auto type = context.type_of_target();
      if (!type.is_array()) {
         context.report_error("applied to a scalar type; only arrays of [arrays of [...]] single-byte values may be marked as strings");
      } else {
         gw::type::array array_type = type.as_array();
         gw::type::base  value_type = array_type.value_type();
         while (value_type.is_array()) {
            array_type = value_type.as_array();
            value_type = array_type.value_type();
         }
         auto extent_opt = array_type.extent();
         if (!extent_opt.has_value()) {
            context.report_error("applied to a variable-length array; VLAs are not supported");
         } else {
            size_t extent = *extent_opt;
            if (extent == 0) {
               context.report_error("applied to a zero-length array");
            } else {
               bool nonstring = false;
               {
                  auto list = context.get_existing_attributes();
                  nonstring = list.has_attribute("nonstring") || list.has_attribute("lu_nonstring");
               }
               if (!nonstring && extent == 1) {
                  context.report_error("applied to a zero-length string (array length is 1; terminator byte is required; ergo no room for any actual string content)");
               }
            }
         }
      }
      
      if (context.has_any_errors()) {
         *no_add_attrs = true;
      }
      return NULL_TREE;
   }
}