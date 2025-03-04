#include "attribute_handlers/helpers/bp_attr_context.h"
#include <array>
#include <utility> // std::pair
#include "lu/stringf.h"
#include <stringpool.h> // dependency of <attribs.h>
#include <attribs.h>
#include "gcc_wrappers/decl/type_def.h"
#include "gcc_wrappers/decl/variable.h"
#include "gcc_wrappers/type/array.h"
#include "basic_global_state.h"
#include "debugprint.h"
namespace gw {
   using namespace gcc_wrappers;
}

namespace attribute_handlers::helpers {
   bp_attr_context::bp_attr_context(tree* node_ptr, tree attribute_name, int flags) {
      assert(node_ptr != nullptr);
      this->attribute_target = node_ptr;
      this->attribute_name   = attribute_name;
      this->attribute_flags  = flags;
      
      tree node = *node_ptr;
      assert(node != NULL_TREE);
      if (DECL_P(node)) {
         switch (TREE_CODE(node)) {
            case TYPE_DECL:
            case FIELD_DECL:
            case VAR_DECL:
               break;
            default:
               report_error("cannot be applied to this kind of declaration; use it on types, variables, or struct/union fields");
               break;
         }
      }
   }
   
   const std::string& bp_attr_context::_get_context_string() {
      auto& str = this->printable;
      if (str.empty()) {
         auto node = this->attribute_target[0];
         
         gw::type::optional_base type;
         gw::decl::optional_base decl;
         if (gw::type::base::raw_node_is(node)) {
            type = node;
         } else if (gw::decl::base::raw_node_is(node)) {
            decl = node;
            type = TREE_TYPE(node);
         }
         
         if (decl) {
            if (decl->is<gw::decl::type_def>()) {
               str = lu::stringf("(applied to type %<%s%>)", decl->name().data());
            } else {
               str = lu::stringf("(applied to field %<%s%>)", decl->name().data());
            }
         } else if (type) {
            auto pp = type->pretty_print();
            str = lu::stringf("(applied to type %<%s%>)", pp.c_str());
         }
      }
      return str;
   }
   
   void bp_attr_context::_mark_with_named_error_attribute() {
      constexpr const char* const sentinel_name = "lu bitpack invalid attribute name";
      
      bool here = false;
      auto list = get_existing_attributes();
      for(auto attr : list) {
         if (attr.name() != sentinel_name)
            continue;
         auto name = attr.arguments().front();
         if (name.code() != IDENTIFIER_NODE)
            continue;
         if (name.unwrap() == this->attribute_name) {
            here = true;
            break;
         }
      }
      if (here)
         return;
      
      auto attr_args = tree_cons(NULL_TREE, this->attribute_name, NULL_TREE);
      auto attr_node = tree_cons(get_identifier(sentinel_name), attr_args, NULL_TREE);
      auto deferred  = decl_attributes(
         this->attribute_target,
         attr_node,
         this->attribute_flags | ATTR_FLAG_INTERNAL,
         this->attribute_target[1]
      );
      assert(deferred == NULL_TREE);
   }
   void bp_attr_context::_mark_with_error_attribute() {
      constexpr const char* const sentinel_name = "lu bitpack invalid attributes";
      
      this->failed = true;
      
      if (get_existing_attributes().has_attribute(sentinel_name))
         return;
      
      auto attr_node = tree_cons(get_identifier(sentinel_name), NULL_TREE, NULL_TREE);
      auto deferred  = decl_attributes(
         this->attribute_target,
         attr_node,
         this->attribute_flags | ATTR_FLAG_INTERNAL,
         this->attribute_target[1]
      );
      assert(deferred == NULL_TREE);
      assert(get_existing_attributes().has_attribute(sentinel_name));
   }
   
   bool bp_attr_context::attribute_already_applied() const {
      return get_existing_attributes().has_attribute(this->attribute_name);
   }
   bool bp_attr_context::attribute_already_attempted() const {
      auto list = get_existing_attributes();
      for(auto attr : list) {
         auto name = attr.name();
         if (name == IDENTIFIER_POINTER(this->attribute_name))
            return true;
         if (name != "lu bitpack invalid attribute name")
            continue;
         auto id_node = attr.arguments().front().unwrap();
         assert(id_node != NULL_TREE && TREE_CODE(id_node) == IDENTIFIER_NODE);
         if (id_node == this->attribute_name)
            return true;
      }
      return false;
   }
   
   gw::attribute_list bp_attr_context::get_existing_attributes() const {
      auto node = this->attribute_target[0];
      assert(node != NULL_TREE);
      if (gw::decl::base::raw_node_is(node))
         return gw::decl::base::wrap(node).attributes();
      if (gw::type::base::raw_node_is(node))
         return gw::type::base::wrap(node).attributes();
      assert(false && "unreachable");
   }
   
   void bp_attr_context::check_and_report_applied_to_integral() {
      auto   type     = type_of_target();
      bool   is_array = type.is_array();
      size_t rank     = 0;
      while (type.is_array()) {
         ++rank;
         type = type.as_array().value_type();
      }
      if (type.is_boolean() || type.is_enum() || type.is_integer()) {
         return;
      }
      auto pp = type.pretty_print();
      if (is_array) {
         if (rank > 1) {
            report_error("applied to multi-dimensional array of a non-integral type %qs", pp.c_str());
         } else {
            report_error("applied to an array of non-integral type %qs", pp.c_str());
         }
      } else {
         report_error("applied to non-integral type %qs", pp.c_str());
      }
   }
   
   bool bp_attr_context::_impl_check_x_options(tree subject, x_option_type here) {
      constexpr const auto attribs_buffer = std::array{
         "lu_bitpack_as_opaque_buffer",
      };
      constexpr const auto attribs_integral = std::array{
         "lu_bitpack_bitcount",
         "lu_bitpack_range",
      };
      constexpr const auto attribs_string = std::array{
         "lu_bitpack_string",
      };
      constexpr const auto attribs_transforms = std::array{
         "lu_bitpack_transforms",
      };
      
      gw::attribute_list list;
      if (gw::type::base::raw_node_is(subject))
         list = gw::type::base::wrap(subject).attributes();
      else
         list = gw::decl::base::wrap(subject).attributes();
      
      auto _has = [&list](std::string_view name) {
         for(auto attr : list) {
            if (attr.name() == name)
               return true;
            if (attr.name() == "lu bitpack invalid attribute name") {
               auto id_node = attr.arguments().front().unwrap();
               assert(id_node != NULL_TREE && TREE_CODE(id_node) == IDENTIFIER_NODE);
               if (IDENTIFIER_POINTER(id_node) == name)
                  return true;
            }
         }
         return false;
      };
      
      if (here != x_option_type::buffer)
         for(const char* name : attribs_buffer)
            if (_has(name))
               return true;
      if (here != x_option_type::integral)
         for(const char* name : attribs_integral)
            if (_has(name))
               return true;
      if (here != x_option_type::string)
         for(const char* name : attribs_string)
            if (_has(name))
               return true;
      if (here != x_option_type::transforms)
         for(const char* name : attribs_transforms)
            if (_has(name))
               return true;
            
      return false;
   }
   void bp_attr_context::check_and_report_contradictory_x_options(x_option_type here) {
      auto type = target_type();
      if (!type) {
         if (_impl_check_x_options(this->attribute_target[0], here)) {
            report_error("conflicts with attributes previously applied to this field");
            return;
         }
         type = type_of_target();
      }
      if (type) {
         if (_impl_check_x_options(type.unwrap(), here)) {
            report_error("conflicts with attributes previously applied to this type");
            return;
         }
         while (type->is_array()) {
            type = type->as_array().value_type();
            if (_impl_check_x_options(type.unwrap(), here)) {
               report_error("conflicts with attributes previously applied to this array type or its value type(s)");
               return;
            }
         }
         return;
      }
   }
   
   gw::decl::optional_field bp_attr_context::target_field() const {
      tree node = this->attribute_target[0];
      if (gw::decl::field::raw_node_is(node))
         return node;
      return {};
   }
   gw::decl::optional_type_def bp_attr_context::target_typedef() const {
      tree node = this->attribute_target[0];
      if (gw::decl::type_def::raw_node_is(node))
         return node;
      return {};
   }
   gw::type::optional_base bp_attr_context::target_type() const {
      tree node = this->attribute_target[0];
      if (gw::type::base::raw_node_is(node))
         return node;
      return {};
   }
   
   gcc_wrappers::type::base bp_attr_context::type_of_target() const {
      auto node = this->attribute_target[0];
      if (gw::type::base::raw_node_is(node))
         return gw::type::base::wrap(node);
      assert(gw::decl::base::raw_node_is(node));
      switch (TREE_CODE(node)) {
         case TYPE_DECL:
            //
            // When attribute handlers are being invoked on a TYPE_DECL, 
            // DECL_ORIGINAL_TYPE hasn't been set yet; the original type 
            // is instead in TREE_TYPE.
            //
            return gw::type::base::wrap(TREE_TYPE(node));
         case FIELD_DECL:
            return gw::decl::field::wrap(node).value_type();
         case VAR_DECL:
            return gw::decl::variable::wrap(node).value_type();
         default:
            break;
      }
      assert(false && "unreachable");
   }
   
   void bp_attr_context::reapply_with_new_args(gcc_wrappers::optional_list_node args) {
      auto attr_node = tree_cons(this->attribute_name, args.unwrap(), NULL_TREE);
      decl_attributes(
         this->attribute_target,
         attr_node,
         this->attribute_flags | ATTR_FLAG_INTERNAL,
         this->attribute_target[1]
      );
   }
   
   void bp_attr_context::add_internal_attribute(lu::strings::zview attr_name, gw::optional_list_node args) {
      auto attr_node = tree_cons(get_identifier(attr_name.c_str()), args.unwrap(), NULL_TREE);
      decl_attributes(
         this->attribute_target,
         attr_node,
         this->attribute_flags | ATTR_FLAG_INTERNAL,
         this->attribute_target[1]
      );
   }
   
   void bp_attr_context::fail_silently() {
      this->_mark_with_error_attribute();
      this->_mark_with_named_error_attribute();
   }
}