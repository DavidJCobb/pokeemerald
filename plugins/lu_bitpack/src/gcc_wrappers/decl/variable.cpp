#include "gcc_wrappers/decl/variable.h"
#include <cassert>
#include "gcc_wrappers/identifier.h"
#include "gcc_wrappers/_node_boilerplate-impl.define.h"
#include <c-tree.h> // C_DECL_DECLARED_CONSTEXPR, c_bind
#include <c-family/c-common.h> // add_stmt
#include <toplev.h> // rest_of_decl_compilation
#include "gcc_headers/plugin-version.h"
#include "gcc_wrappers/environment/c/constexpr_supported.h"
#include "gcc_wrappers/environment/c/dialect.h"

namespace gcc_wrappers::decl {
   GCC_NODE_WRAPPER_BOILERPLATE(variable)
   
   variable::variable(
      lu::strings::zview identifier_name,
      type::base         variable_type,
      location_t         source_location
   ) {
      auto identifier_node = identifier(identifier_name);
      this->_node = build_decl(
         source_location,
         VAR_DECL,
         identifier_node.unwrap(),
         variable_type.unwrap()
      );
   }
   variable::variable(
      identifier  identifier_node,
      type::base  variable_type,
      location_t  source_location
   ) {
      this->_node = build_decl(
         source_location,
         VAR_DECL,
         identifier_node.unwrap(),
         variable_type.unwrap()
      );
   }
   
   optional_value variable::initial_value() const {
      return DECL_INITIAL(this->_node);
   }
   void variable::set_initial_value(optional_value v) {
      DECL_INITIAL(this->_node) = v.unwrap();
   }
   
   expr::declare variable::make_declare_expr() {
      return expr::declare(*this);
   }
   
   bool variable::is_defined_elsewhere() const {
      return DECL_EXTERNAL(this->_node);
   }
   void variable::set_is_defined_elsewhere(bool v) {
      DECL_EXTERNAL(this->_node) = v ? 1 : 0;
   }
   
   bool variable::is_externally_accessible() const {
      return TREE_PUBLIC(this->_node);
   }
   void variable::make_externally_accessible() {
      set_is_externally_accessible(true);
   }
   void variable::set_is_externally_accessible(bool v) {
      TREE_PUBLIC(this->_node) = v ? 1 : 0;
   }
   
   bool variable::is_ever_read() const {
      return DECL_READ_P(this->_node);
   }
   void variable::make_ever_read() {
      set_is_ever_read(true);
   }
   void variable::set_is_ever_read(bool v) {
      DECL_READ_P(this->_node) = v ? 1 : 0;
   }
   
   bool variable::is_declared_constexpr() const {
      #if GCCPLUGIN_VERSION_MAJOR >= 13 && GCCPLUGIN_VERSION_MINOR >= 1
         return C_DECL_DECLARED_CONSTEXPR(this->_node);
      #else
         return false;
      #endif
   }
   void variable::make_declared_constexpr() {
      set_is_declared_constexpr(true);
   }
   void variable::set_is_declared_constexpr(bool v) {
      {
         using namespace environment::c;
         assert(constexpr_supported);
         if (v) {
            assert(current_dialect() >= dialect::c23);
         }
      }
      #if GCCPLUGIN_VERSION_MAJOR >= 13 && GCCPLUGIN_VERSION_MINOR >= 1
         C_DECL_DECLARED_CONSTEXPR(this->_node) = v ? 1 : 0;
      #endif
   }
   
   // https://gcc.gnu.org/pipermail/gcc/2022-November/240196.html
   void variable::make_file_scope_extern() {
      TREE_STATIC(this->_node) = 1;
      c_bind(DECL_SOURCE_LOCATION(this->_node), this->_node, true); // c/c-tree.h
      rest_of_decl_compilation(this->_node, true, false); // toplev.h
   }
   
   void variable::commit_to_current_scope() {
      assert(DECL_CONTEXT(this->_node) == NULL_TREE);
      //
      // The `c_bind` API binds a variable to either function-scope or 
      // file-scope.
      //
      if (current_function_decl == NULL_TREE) {
         TREE_STATIC(this->_node) = 1;
      }
      c_bind(DECL_SOURCE_LOCATION(this->_node), this->_node, false); // c/c-tree.h
      rest_of_decl_compilation(this->_node, true, false); // toplev.h
      if (!DECL_FILE_SCOPE_P(this->_node)) {
         add_stmt(make_declare_expr().unwrap());
      }
   }
}