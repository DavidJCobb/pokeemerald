#include "attribute_handlers/bitpack_transforms.h"
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <c-family/c-common.h> // lookup_name
#include "lu/strings/handle_kv_string.h"
#include "attribute_handlers/helpers/bp_attr_context.h"
#include "attribute_handlers/generic_bitpacking_data_option.h"
#include "bitpacking/transform_function_validation_helpers.h"
#include "gcc_wrappers/constant/integer.h"
#include "gcc_wrappers/constant/string.h"
#include "gcc_wrappers/decl/function.h"
#include "gcc_wrappers/builtin_types.h"
#include "gcc_wrappers/identifier.h"
namespace gw {
   using namespace gcc_wrappers;
}

static std::string _name_to_report(gw::type::base type) {
   std::string out;
   auto name = type.name();
   if (type.declaration()) {
      out = name;
   } else {
      if (type.is_record()) {
         out += "struct ";
      } else if (type.is_union()) {
         out += "union ";
      }
      out += name;
   }
   return out;
}

namespace attribute_handlers {
   static void _reassemble_parameters(
      gw::constant::string      data,
      helpers::bp_attr_context& context
   ) {
      struct {
         std::optional<std::string> pre_pack;
         std::optional<std::string> post_unpack;
         bool never_split_across_sectors = false;
      } params;
      try {
         lu::strings::handle_kv_string(data.value(), std::array{
            lu::strings::kv_string_param{
               .name      = "pre_pack",
               .has_param = true,
               .handler   = [&params](std::string_view v, int vi) {
                  params.pre_pack = v;
               },
            },
            lu::strings::kv_string_param{
               .name      = "post_unpack",
               .has_param = true,
               .handler   = [&params](std::string_view v, int vi) {
                  params.post_unpack = v;
               },
            },
            lu::strings::kv_string_param{
               .name      = "never_split_across_sectors",
               .has_param = false,
               .handler   = [&params](std::string_view v, int vi) {
                  params.never_split_across_sectors = true;
               },
            },
         });
      } catch (std::runtime_error& ex) {
         std::string message = "argument failed to parse: ";
         message += ex.what();
         context.report_error(message);
         return;
      }
      
      bool both_are_individually_correct = true;
      
      if (!params.pre_pack.has_value()) {
         context.report_error("does not specify a pre-pack function name");
         both_are_individually_correct = false;
      }
      if (!params.post_unpack.has_value()) {
         context.report_error("does not specify a post-unpack function name");
         both_are_individually_correct = false;
      }
      
      gw::decl::optional_function pre_pack;
      gw::decl::optional_function post_unpack;
      
      auto _get_func = [&context](
         const char* what,
         const std::optional<std::string>& src_opt,
         gw::decl::optional_function& dst
      ) {
         if (!src_opt.has_value())
            return;
         const auto& src = *src_opt;
         if (src.empty()) {
            context.report_error("the %s function name, if specified, cannot be blank", what);
            return;
         }
         auto id   = gw::identifier(src.c_str());
         auto node = lookup_name(id.unwrap());
         if (node != NULL_TREE && TREE_CODE(node) == ADDR_EXPR) {
            //
            // Allow `&func`.
            //
            node = TREE_OPERAND(node, 0);
         }
         if (node == NULL_TREE) {
            context.report_error("specifies an identifier for the %s function, %qE, that does not exist", what, id.unwrap());
            return;
         }
         if (!gw::decl::function::raw_node_is(node)) {
            context.report_error("specifies an identifier for the %s function, %qE, that does not refer to a function declaration", what, id.unwrap());
            return;
         }
         dst = node;
      };
      
      _get_func("pre-pack",    params.pre_pack,    pre_pack);
      _get_func("post-unpack", params.post_unpack, post_unpack);
      
      gw::type::base target_type = context.type_of_target();
      
      if (pre_pack && !bitpacking::can_be_pre_pack_function(*pre_pack)) {
         auto name = pre_pack->name().data();
         auto type = target_type.name();
         if (!type.empty()) {
            auto for_sig = _name_to_report(target_type);
            context.report_error("specifies a pre-pack function %<%s%> with the wrong signature (should be %<void %s(const %s*, PackedType*)%> given some type %<PackedType%>)", name, name, for_sig.c_str());
         } else {
            context.report_error("specifies a pre-pack function %<%s%> with the wrong signature (should be %<void %s(const T*, PackedType*)%> given some type %<%T%> and some type %<PackedType%>)", name, name);
         }
         both_are_individually_correct = false;
      }
      if (post_unpack && !bitpacking::can_be_post_unpack_function(*post_unpack)) {
         auto name = pre_pack->name().data();
         auto type = target_type.name();
         if (!type.empty()) {
            auto for_sig = _name_to_report(target_type);
            context.report_error("specifies a post-unpack function %<%s%> with the wrong signature (should be %<void %s(%s*, const PackedType*)%> given some type %<PackedType%>)", name, name, for_sig.c_str());
         } else {
            context.report_error("specifies a post-unpack function %<%s%> with the wrong signature (should be %<void %s(T*, const PackedType*)%> given some type %<%T%> and some type %<PackedType%>)", name, name);
         }
         both_are_individually_correct = false;
      }
      
      if (pre_pack && post_unpack && both_are_individually_correct) {
         //
         // Verify that the functions match each other.
         //
         auto mismatch_text = bitpacking::check_transform_functions_match_types(*pre_pack, *post_unpack);
         if (!mismatch_text.empty()) {
            mismatch_text.insert(0, "specifies incompatible functions: ");
            context.report_error(mismatch_text);
         }
         //
         // If the functions' in situ types match, verify that they match 
         // the thing we're applying this attribute to.
         //
         auto in_situ = bitpacking::get_in_situ_type(*pre_pack, *post_unpack);
         if (in_situ) {
            if (in_situ->main_variant() != target_type.main_variant()) {
               auto pp_a = in_situ->pretty_print();
               auto pp_b = target_type.pretty_print();
               context.report_error("specifies functions that transform type %<%s%>, but would be applied to type %<%s%>", pp_a.c_str(), pp_b.c_str());
            }
         }
      }
      
      if (context.has_any_errors()) {
         return;
      }
      
      gw::list_node args({}, pre_pack);
      args.append({}, post_unpack);
      {
         auto cst = gw::constant::integer(
            gw::builtin_types::get().basic_int,
            params.never_split_across_sectors ? 1 : 0
         );
         args.append({}, cst);
      }
      context.reapply_with_new_args(args);
   }

   extern tree bitpack_transforms(tree* node_ptr, tree name, tree args, int flags, bool* no_add_attrs) {
      *no_add_attrs = false;
      
      auto result = generic_bitpacking_data_option(node_ptr, name, args, flags, no_add_attrs);
      if (*no_add_attrs) {
         return result;
      }
      
      if (flags & ATTR_FLAG_INTERNAL) {
         return NULL_TREE;
      }
      if (list_length(args) > 1) {
         error("wrong number of arguments specified for %qE attribute", name);
         *no_add_attrs = true;
         return NULL_TREE;
      }
      
      helpers::bp_attr_context context(node_ptr, name, flags);
      context.check_and_report_contradictory_x_options(helpers::x_option_type::transforms);
      
      /*//
      //
      // Checked when the containing type finishes parsing, so we can 
      // catch bitfields that are affected by the attribute by virtue 
      // of their value types.
      //
      {
         auto field = context.target_field();
         if (!field.empty() && field.is_non_addressable()) {
            context.report_error("applied to a non-addressable object (bit-fields, etc.)");
         }
      }
      //*/
      
      gw::constant::optional_string data;
      {
         auto next = TREE_VALUE(args);
         if (next != NULL_TREE && gw::constant::string::raw_node_is(next)) {
            data = next;
         } else {
            context.report_error("argument, if specified, must be a string constant");
         }
      }
      
      *no_add_attrs = true;
      if (data) {
         //
         // Parse the arguments and reconstruct them.
         //
         _reassemble_parameters(*data, context);
      }
      return NULL_TREE;
   }
}