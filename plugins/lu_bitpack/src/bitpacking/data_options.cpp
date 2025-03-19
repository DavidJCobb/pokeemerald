#include <cassert>
#include "bitpacking/data_options.h"
#include "attribute_handlers/helpers/type_transitively_has_attribute.h"
#include "bitpacking/for_each_influencing_entity.h"
#include "bitpacking/transform_function_validation_helpers.h" // get_transformed_type
#include "bitpacking/verify_union_internal_tag.h"
#include "bitpacking/verify_union_members.h"
#include "basic_global_state.h"
#include "gcc_wrappers/constant/integer.h"
#include "gcc_wrappers/constant/string.h"
#include "gcc_wrappers/decl/field.h"
#include "gcc_wrappers/decl/param.h"
#include "gcc_wrappers/decl/variable.h"
#include "gcc_wrappers/type/array.h"
#include "gcc_wrappers/type/base.h"
#include "gcc_wrappers/attribute.h"
#include "gcc_wrappers/attribute_list.h"
#include "gcc_wrappers/identifier.h"
#include <diagnostic.h>
namespace gw {
   using namespace gcc_wrappers;
}

static gw::type::optional_base _get_innermost_value_type(gw::node node) {
   gw::type::optional_base value_type;
   if (node.is<gw::decl::base_value>()) {
      value_type = node.as<gw::decl::base_value>().value_type();
   } else if (node.is<gw::type::base>()) {
      value_type = node.as<gw::type::base>();
   }
   if (value_type) {
      while (value_type->is_array())
         value_type = value_type->as_array().value_type();
   }
   return value_type;
}

static location_t _loc(gw::node node) {
   if (node.is<gw::decl::base>()) {
      return node.as<gw::decl::base>().source_location();
   } else if (node.is<gw::type::base>()) {
      return node.as<gw::type::base>().source_location();
   }
   return UNKNOWN_LOCATION;
}

namespace bitpacking {
   void data_options::_report_typed_load_failure(gw::node target, gw::node node, std::string_view desired_type) {
      if (this->_reported_typed_load_failure)
         return;
      this->_reported_typed_load_failure = true;
      if (!this->config.report_errors)
         return;
      
      std::string target_name;
      std::string node_name;
      if (target.is<gw::decl::base>()) {
         target_name = target.as<gw::decl::base>().name();
      } else if (target.is<gw::type::base>()) {
         target_name = target.as<gw::type::base>().name();
      }
      if (node.is<gw::decl::base>()) {
         node_name = node.as<gw::decl::base>().name();
      } else if (node.is<gw::type::base>()) {
         node_name = node.as<gw::type::base>().name();
      }
      
      if (target_name.empty())
         target_name = "???";
      if (node_name.empty())
         node_name = "???";
      
      std::string_view seen_type = "???";
      {
         const auto& var = std::get<typed_data_options::requested_variant>(this->typed);
         if (std::holds_alternative<typed_data_options::requested::buffer>(var))
            seen_type = "buffer";
         else if (std::holds_alternative<typed_data_options::requested::integral>(var))
            seen_type = "integral";
         else if (std::holds_alternative<typed_data_options::requested::string>(var))
            seen_type = "string";
         else if (std::holds_alternative<typed_data_options::requested::transformed>(var))
            seen_type = "transformed";
         else if (std::holds_alternative<typed_data_options::requested::tagged_union>(var))
            seen_type = "union";
      }
      error_at(_loc(target), "bitpacking options specified on %<%s%>, affecting %<%s%>, conflict with previously-seen options: we have just seen %s options, but previously saw %s options (further errors of this variety will not be reported for %<%s%>)", target_name.data(), node_name.data(), desired_type.data(), seen_type.data(), target_name.data());
   }
   
   void data_options::_load_contributing_entity(gw::node target, gw::node node, gw::attribute_list attributes) {
      if (!this->has_attr_nonstring) {
         gw::type::optional_base type;
         if (node.is<gw::decl::base_value>()) {
            if (attributes.has_attribute("lu_nonstring") || attributes.has_attribute("nonstring")) {
               this->has_attr_nonstring = true;
            } else {
               type = node.as<gw::decl::base_value>().value_type();
            }
         } else if (node.is<gw::type::base>()) {
            type = node.as<gw::type::base>();
         }
         if (type) {
            //
            // Search this type for the attributes. Note that GCC doesn't 
            // allow `nonstring` on type(def)s.
            //
            if (attribute_handlers::helpers::type_transitively_has_attribute(*type, "lu_nonstring"))
               this->has_attr_nonstring = true;
         }
      }
      
      for(auto attr : attributes) {
         auto key = attr.name();
         if (key == "lu bitpack invalid attributes") {
            this->_failed = true;
            continue;
         }
         
         //
         // Attribute arguments will have been validated in advance: GCC 
         // verified that the attributes have the right number of args, 
         // and our attribute handlers rejected any obviously malformed 
         // args. We can just access things blindly.
         //
         
         if (key == "lu_bitpack_omit") {
            this->is_omitted = true;
            continue;
         }
         if (key == "lu_bitpack_default_value") {
            this->default_value = attr.arguments().front();
            continue;
         }
         if (key == "lu_bitpack_union_member_id") {
            this->union_member_id = attr.arguments().front().as<gw::constant::integer>().value<intmax_t>();
            continue;
         }
         if (key == "lu_bitpack_stat_category") {
            auto str = attr.arguments().front().as<gw::constant::string>();
            this->stat_categories.push_back(std::string(str.value()));
            continue;
         }
         if (key == "lu_bitpack_misc_annotation") {
            auto str = attr.arguments().front().as<gw::constant::string>();
            this->misc_annotations.push_back(std::string(str.value()));
            continue;
         }
         
         //
         // Typed data options:
         //
         
         // Buffer:
         if (key == "lu_bitpack_as_opaque_buffer") {
            if (this->_fail_if_cannot_load_as<typed_data_options::requested::buffer>(target, node, "buffer"))
               continue;
            _get_or_emplace_for_load<typed_data_options::requested::buffer>();
            continue;
         }
         
         // Integral:
         if (key == "lu_bitpack_bitcount") {
            if (this->_fail_if_cannot_load_as<typed_data_options::requested::integral>(target, node, "integral"))
               continue;
            auto& dst = _get_or_emplace_for_load<typed_data_options::requested::integral>();
            dst.bitcount = attr.arguments().front().as<gw::constant::integer>().value<intmax_t>();
            continue;
         }
         if (key == "lu_bitpack_range") {
            if (this->_fail_if_cannot_load_as<typed_data_options::requested::integral>(target, node, "integral"))
               continue;
            auto& dst  = _get_or_emplace_for_load<typed_data_options::requested::integral>();
            auto  args = attr.arguments();
            dst.min = args[0].as<gw::constant::integer>().value<intmax_t>();
            dst.max = args[1].as<gw::constant::integer>().value<intmax_t>();
            continue;
         }
         
         // String:
         if (key == "lu_bitpack_string") {
            if (this->_fail_if_cannot_load_as<typed_data_options::requested::string>(target, node, "string"))
               continue;
            _get_or_emplace_for_load<typed_data_options::requested::string>();
            continue;
         }
         
         // Tagged union:
         if (key == "lu_bitpack_union_external_tag") {
            if (this->_fail_if_cannot_load_as<typed_data_options::requested::tagged_union>(target, node, "union"))
               continue;
            auto& dst = _get_or_emplace_for_load<typed_data_options::requested::tagged_union>();
            dst.tag_identifier = attr.arguments().front().as<gw::identifier>().name();
            dst.is_external    = true;
            continue;
         }
         if (key == "lu_bitpack_union_internal_tag") {
            if (this->_fail_if_cannot_load_as<typed_data_options::requested::tagged_union>(target, node, "union"))
               continue;
            auto& dst = _get_or_emplace_for_load<typed_data_options::requested::tagged_union>();
            dst.tag_identifier = attr.arguments().front().as<gw::identifier>().name();
            dst.is_internal    = true;
            continue;
         }
         
         // Transformed:
         if (key == "lu_bitpack_transforms") {
            if (this->_fail_if_cannot_load_as<typed_data_options::requested::transformed>(target, node, "transformed"))
               continue;
            auto& dst  = _get_or_emplace_for_load<typed_data_options::requested::transformed>();
            auto  args = attr.arguments();
            dst.pre_pack    = args[0].as<gw::decl::function>();
            dst.post_unpack = args[1].as<gw::decl::function>();
            if (dst.pre_pack && dst.post_unpack) {
               dst.transformed_type = get_transformed_type(*dst.pre_pack, *dst.post_unpack);
            }
            dst.never_split_across_sectors = args[2].as<gw::constant::integer>().try_value_unsigned().value() != 0;
            continue;
         }
         
         if (key.starts_with("lu_bitpack_")) {
            std::string_view noun = "identifier";
            std::string_view name;
            location_t       loc = UNKNOWN_LOCATION;
            if (node.is<gw::type::base>()) {
               auto casted = node.as<gw::type::base>();
               loc  = casted.source_location();
               name = casted.name();
               noun = "type";
            } else if (node.is<gw::decl::base>()) {
               auto casted = node.as<gw::decl::base>();
               loc  = casted.source_location();
               name = casted.name();
               noun = "declaration";
            }
            warning_at(loc, OPT_Wpragmas, "%s %<%s%> has unrecognized bitpacking attribute %<%s%>", noun.data(), name.data(), key.data());
         }
      }
   }
   
   void data_options::_finalize(gcc_wrappers::node node) {
      if (node.is<gw::type::base>()) {
         auto type = node.as<gw::type::base>();
         this->_validate_union(type);
      } else if (node.is<gw::decl::base_value>()) {
         auto type = node.as<gw::decl::base_value>().value_type();
         this->_validate_union(type);
      }
      
      this->_loaded = true;
      if (this->_failed) {
         return;
      }
      
      //
      // If nothing went wrong, then compute the final bitpacking options.
      //
      
      // Intentional copy.
      const auto loaded  = std::get<typed_data_options::requested_variant>(this->typed);
      auto&      dst_var = this->typed.emplace<typed_data_options::computed_variant>();
      
      if (auto* casted = std::get_if<typed_data_options::requested::buffer>(&loaded)) {
         auto& dst = dst_var.emplace<typed_data_options::computed::buffer>();
         if (!dst.load(node, *casted, this->config.report_errors)) {
            this->_failed = true;
         }
      } else if (auto* casted = std::get_if<typed_data_options::requested::integral>(&loaded)) {
         auto& dst = dst_var.emplace<typed_data_options::computed::integral>();
         if (!dst.load(node, *casted, this->config.report_errors)) {
            this->_failed = true;
         }
      } else if (auto* casted = std::get_if<typed_data_options::requested::string>(&loaded)) {
         auto& dst = dst_var.emplace<typed_data_options::computed::string>();
         dst.nonstring = this->has_attr_nonstring;
         if (!dst.load(node, *casted, this->config.report_errors)) {
            this->_failed = true;
         }
      } else if (auto* casted = std::get_if<typed_data_options::requested::transformed>(&loaded)) {
         auto& dst = dst_var.emplace<typed_data_options::computed::transformed>();
         if (!dst.load(node, *casted, this->config.report_errors)) {
            this->_failed = true;
         }
      } else if (auto* casted = std::get_if<typed_data_options::requested::tagged_union>(&loaded)) {
         auto& dst = dst_var.emplace<typed_data_options::computed::tagged_union>();
         if (!dst.load(node, *casted, this->config.report_errors)) {
            this->_failed = true;
         }
      } else {
         //
         // No typed bitpacking options were specified, so we should try to 
         // determine them automatically.
         //
         if (!this->is_omitted) {
            //
            // ...if this entity isn't omitted from bitpacking, anyway.
            //
            auto value_type = _get_innermost_value_type(node);
            if (!value_type) {
               if (this->config.report_errors) {
                  error_at(_loc(node), "failed to apply bitpacking options: we cannot figure out what value type we are applying them to, for some reason, and the options that were specified were not specific enough to work despite that");
               }
               this->_failed = true;
            } else {
               //
               // We'll determine how to pack the value based on its type.
               //
               if (value_type->is_integral()) {
                  //
                  // Integral bitpacking options already cope with the user 
                  // specifying only some of the available options, so we 
                  // can just pass in none of the available options.
                  //
                  const typed_data_options::requested::integral empty;
                  //
                  auto& dst = dst_var.emplace<typed_data_options::computed::integral>();
                  if (!dst.load(node, empty, this->config.report_errors)) {
                     this->_failed = true;
                  }
               } else if (value_type->is_floating_point()) {
                  //
                  // Just pack floats as opaque buffers for now.
                  //
                  auto& dst = dst_var.emplace<typed_data_options::computed::buffer>();
                  dst.bytecount = value_type->size_in_bytes();
               } else if (value_type->is_pointer()) {
                  //
                  // There are no bitpacking options that you can specify on
                  // a pointer, but pointers can be bitpacked (as opaque, 
                  // basically).
                  //
                  dst_var.emplace<typed_data_options::computed::pointer>();
               } else if (value_type->is_record()) {
                  dst_var.emplace<typed_data_options::computed::structure>();
               } else {
                  //
                  // Unrecognized type.
                  //
                  if (this->config.report_errors) {
                     auto loc = _loc(node);
                     error_at(loc, "failed to apply bitpacking options: this is not a type we know how to handle (if %<memcpy%> is fine, pack it as an opaque buffer)");
                     inform(loc, "if this is a value that can be safely memcpy%'d, then mark it for packing as an opaque buffer");
                  }
                  this->_failed = true;
               }
            }
         }
      }
      //
      // One final case: booleans.
      //
      if (!this->_failed) {
         if (auto* casted = std::get_if<typed_data_options::computed::integral>(&dst_var)) {
            if (
               (casted->min == 0 || casted->min == typed_data_options::computed::integral::no_minimum) &&
               (casted->max == 1 || casted->max == typed_data_options::computed::integral::no_maximum)
            ) {
               //
               // This integral value looks like a boolean.
               //
               auto value_type = _get_innermost_value_type(node);
               if (value_type) {
                  if (basic_global_state::get().global_options.type_is_boolean(*value_type)) {
                     //
                     // Booleans have their own bitstream function that we want 
                     // to prefer.
                     //
                     dst_var.emplace<typed_data_options::computed::boolean>();
                  }
               }
            }
         }
      }
   }
   
   void data_options::_validate_union(gw::type::base type) {
      gw::type::optional_untagged_union union_type;
      {
         auto here = type;
         while (here.is_array())
            here = here.as_array().value_type();
         if (here.is_union())
            union_type = here.as_union();
      }
      
      auto& outer  = std::get<typed_data_options::requested_variant>(this->typed);
      auto* casted = std::get_if<typed_data_options::requested::tagged_union>(&outer);
      if (!casted) {
         if (
            union_type &&
            !std::holds_alternative<typed_data_options::requested::buffer>(outer) &&
            !std::holds_alternative<typed_data_options::requested::transformed>(outer)
         ) {
            //
            // Can't bitpack a union if we don't understand how it's tagged 
            // (and if it's not an opaque buffer or transformed).
            //
            this->_failed = true;
         }
         return;
      }
      
      if (casted->is_external && casted->is_internal) {
         //
         // Contradictory tag (both internally and externally tagged).
         //
         this->_failed = true;
         return;
      }
      
      if (!union_type) {
         //
         // Why are tagged-union options being applied to a non-union?
         //
         this->_failed = true;
         return;
      }
      //
      // Union types have already been validated, but we have no way of marking 
      // types with an error sentinel, so we have to do extra error checking 
      // here.
      //
      if (!verify_union_members(*union_type, true)) {
         this->_failed = true;
         return;
      }
      if (casted->is_internal) {
         if (!verify_union_internal_tag(*union_type, casted->tag_identifier, true)) {
            this->_failed = true;
            return;
         }
      }
   }
   
   [[noreturn]] void data_options::_as_accessor_failed() const {
      internal_error("problem with the bitpacking plug-in: incorrectly-typed access to %<bitpacking::data_options%>");
   }
   
   void data_options::load(gw::decl::field decl) {
      assert(!this->_loaded);
      
      for_each_influencing_entity(decl, [this, decl](gw::node entity) {
         if (entity.is<gw::type::base>()) {
            this->_load_contributing_entity(decl, entity, entity.as<gw::type::base>().attributes());
         } else {
            this->_load_contributing_entity(decl, entity, entity.as<gw::decl::base>().attributes());
         }
      });
      
      this->_finalize(decl);
   }
   void data_options::load(gw::decl::param decl) {
      assert(!this->_loaded);
      // We should only end up querying options for a parameter when we're 
      // dealing with the struct pointer argument for a whole-struct function; 
      // those functions are generated, so the parameters should never have 
      // any arguments. Just act on the type.
      auto type = decl.value_type();
      
      for_each_influencing_entity(type, [this, type](gw::node entity) {
         if (entity.is<gw::type::base>()) {
            this->_load_contributing_entity(type, entity, entity.as<gw::type::base>().attributes());
         } else {
            this->_load_contributing_entity(type, entity, entity.as<gw::decl::base>().attributes());
         }
      });
      
      this->_finalize(type);
   }
   void data_options::load(gw::decl::variable decl) {
      assert(!this->_loaded);
      
      for_each_influencing_entity(decl, [this, decl](gw::node entity) {
         if (entity.is<gw::type::base>()) {
            this->_load_contributing_entity(decl, entity, entity.as<gw::type::base>().attributes());
         } else {
            this->_load_contributing_entity(decl, entity, entity.as<gw::decl::base>().attributes());
         }
      });
      
      this->_finalize(decl);
   }
   void data_options::load(gw::type::base type) {
      assert(!this->_loaded);
      
      for_each_influencing_entity(type, [this, type](gw::node entity) {
         if (entity.is<gw::type::base>()) {
            this->_load_contributing_entity(type, entity, entity.as<gw::type::base>().attributes());
         } else {
            this->_load_contributing_entity(type, entity, entity.as<gw::decl::base>().attributes());
         }
      });
      
      this->_finalize(type);
   }
   
   bool data_options::same_format_as(const data_options& other) const {
      if (this->is_omitted != other.is_omitted)
         return false;
      return this->typed == other.typed;
   }
}