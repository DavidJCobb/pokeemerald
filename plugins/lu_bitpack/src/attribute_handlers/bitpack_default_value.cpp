#include "attribute_handlers/bitpack_default_value.h"
#include <cstdint>
#include "attribute_handlers/helpers/bp_attr_context.h"
#include "attribute_handlers/helpers/type_transitively_has_attribute.h"
#include "attribute_handlers/generic_bitpacking_data_option.h"
#include "gcc_wrappers/constant/string.h"
#include "gcc_wrappers/type/base.h"
#include "gcc_wrappers/type/array.h"
#include "gcc_wrappers/attribute.h"
#include "gcc_wrappers/attribute_list.h"
namespace gw {
   using namespace gcc_wrappers;
}

namespace attribute_handlers {
   static void _check_and_report_real_value(helpers::bp_attr_context& context, tree node) {
      auto type = context.type_of_target();
      while (type.is_array()) {
         type = type.as_array().value_type();
      }
      if (type.is_floating_point()) {
         return;
      }
      //
      // TODO: Allow floating-point constants on bools and integrals, warning if they'd 
      //       be truncated.
      //
      context.report_error("specifies a default value of an incorrect type");
   }
   static void _check_and_report_integral_value(helpers::bp_attr_context& context, tree node) {
      auto type = context.type_of_target();
      while (type.is_array()) {
         type = type.as_array().value_type();
      }
      if (type.is_boolean() || type.is_enum() || type.is_integer()) {
         return;
      }
      if (type.is_pointer()) {
         if (!integer_zerop(node)) {
            context.report_error("uses a non-zero default value; the only permitted default value for a pointer is zero");
         }
         return;
      }
      context.report_error("specifies a default value of an incorrect type");
   }
   static void _check_and_report_string_value(helpers::bp_attr_context& context, tree node) {
      auto type = context.type_of_target();
      if (!type.is_array()) {
         context.report_error("specifies a default value of an incorrect type");
         return;
      }
      bool nonstring = false;
      {
         auto list = context.get_existing_attributes();
         nonstring = list.has_attribute("nonstring") || list.has_attribute("lu_nonstring");
      }
      if (!nonstring)
         nonstring = helpers::type_transitively_has_attribute(type, "lu_nonstring");
      
      gw::type::array array_type = type.as_array();
      gw::type::base  value_type = array_type.value_type();
      while (value_type.is_array()) {
         array_type = value_type.as_array();
         value_type = array_type.value_type();
      }
      //
      // TODO: Fail if the value type isn't a single-byte type
      //
      if (!value_type.is_integer()) {
         context.report_error("applied to an array of [arrays of [...]] non-chars/non-integers");
         return;
      }
      
      auto extent_opt = array_type.extent();
      if (!extent_opt.has_value()) {
         context.report_error("applied to a variable-length array; VLAs are not supported");
         return;
      }
      
      auto str_data = gw::constant::string::wrap(node);
      auto str_size = str_data.size();
      auto extent   = *extent_opt;
      if (nonstring)
         --str_size;
      
      if (str_size > extent) {
         context.report_error(
            "specifies a default value that is too long (size %r%u%R) to fit in the destination (size %r%u%R)",
            "quote",
            (int)str_size,
            "quote",
            (int)extent
         );
         if (!nonstring && str_size == extent + 1) {
            if (!context.target_type()) {
               context.report_note("if these strings do not require null terminators while in memory, then applying %<__attribute__((nonstring))%> before this attribute would allow this value to fit, while also allowing GCC to error-check certain string functions for you. that attribute is only supported on declarations; you may use %<__attribute__((lu_nonstring))%> for declarations or types");
            } else {
               context.report_note("if these strings do not require null terminators while in memory, then applying %<__attribute__((lu_nonstring))%> before this attribute would allow this value to fit");
            }
         }
      }
   }
   
   extern tree bitpack_default_value(tree* node_ptr, tree name, tree args, int flags, bool* no_add_attrs) {
      *no_add_attrs = false;
      
      auto result = generic_bitpacking_data_option(node_ptr, name, args, flags, no_add_attrs);
      if (*no_add_attrs) {
         return result;
      }
      
      helpers::bp_attr_context context(node_ptr, name, flags);
      
      auto value_node = TREE_VALUE(args);
      if (value_node == NULL_TREE) {
         context.report_error("specified with a blank argument");
      } else {
         switch (TREE_CODE(value_node)) {
            case INTEGER_CST:
               _check_and_report_integral_value(context, value_node);
               break;
            case REAL_CST:
               _check_and_report_real_value(context, value_node);
               break;
            case STRING_CST:
               _check_and_report_string_value(context, value_node);
               break;
            default:
               context.report_error("specified with an inappropriate argument");
         }
      }
      
      if (context.has_any_errors()) {
         *no_add_attrs = true;
      }
      return NULL_TREE;
   }
}