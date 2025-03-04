#include "gcc_wrappers/decl/base.h"
#include "lu/strings/builder.h"
#include "gcc_wrappers/decl/field.h"
#include "gcc_wrappers/scope.h"
#include "gcc_wrappers/_node_boilerplate-impl.define.h"
#include <cassert>

namespace gcc_wrappers::decl {
   GCC_NODE_WRAPPER_BOILERPLATE(base)
   
   std::string_view base::name() const {
      auto id_node = DECL_NAME(this->_node);
      if (id_node == NULL_TREE)
         return {};
      return std::string_view(IDENTIFIER_POINTER(id_node));
   }
   std::string base::fully_qualified_name() const {
      constexpr const std::string_view scope_separator = "::";
      
      lu::strings::builder builder;
      {
         auto name = this->name();
         if (!name.empty()) {
            builder.append(name);
         } else {
            bool nameless = true;
            if (this->code() == FIELD_DECL) {
               auto casted = as<field>();
               if (casted.is_class_vtbl_pointer()) {
                  builder.append("<vtbl>");
                  nameless = false;
               }
            }
            if (nameless)
               return {};
         }
      }
      for(auto s = this->context(); s; s = s->containing_scope()) {
         std::string chunk;
         if (s->is_declaration()) {
            auto name = s->as_declaration().name();
            if (name.empty()) {
               break;
            }
            builder.prepend(scope_separator);
            builder.prepend(name);
         } else if (s->is_type()) {
            auto name = s->as_type().pretty_print();
            if (name.empty()) {
               break;
            }
            builder.prepend(scope_separator);
            builder.prepend(name);
         } else {
            break;
         }
      }
      
      return builder.build_and_clear();
   }
   
   attribute_list base::attributes() {
      return attribute_list::wrap(DECL_ATTRIBUTES(this->_node));
   }
   const attribute_list base::attributes() const {
      return attribute_list::wrap(DECL_ATTRIBUTES(this->_node));
   }
   
   location_t base::source_location() const {
      return DECL_SOURCE_LOCATION(this->_node);
   }
   
   std::string_view base::source_file() const {
      return std::string_view(DECL_SOURCE_FILE(this->_node));
   }
   int base::source_line() const {
      return DECL_SOURCE_LINE(this->_node);
   }
   
   bool base::is_at_file_scope() const {
      auto scope = DECL_CONTEXT(this->_node);
      if (scope == NULL_TREE)
         return false;
      return TREE_CODE(scope) == TRANSLATION_UNIT_DECL;
   }
   
   bool base::is_artificial() const {
      return DECL_ARTIFICIAL(this->_node) != 0;
   }
   void base::make_artificial() {
      set_is_artificial(true);
   }
   void base::set_is_artificial(bool v) {
      DECL_ARTIFICIAL(this->_node) = v;
   }
            
   bool base::is_sym_debugger_ignored() const {
      return DECL_IGNORED_P(this->_node) != 0;
   }
   void base::make_sym_debugger_ignored() {
      set_is_sym_debugger_ignored(true);
   }
   void base::set_is_sym_debugger_ignored(bool v) {
      DECL_IGNORED_P(this->_node) = v;
   }
   
   bool base::is_used() const {
      return TREE_USED(this->_node) != 0;
   }
   void base::make_used() {
      set_is_used(true);
   }
   void base::set_is_used(bool v) {
      TREE_USED(this->_node) = v;
   }
   
   optional_scope base::context() const {
      return DECL_CONTEXT(this->_node);
   }
}