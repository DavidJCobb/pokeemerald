#include "gcc_wrappers/scope.h"
#include "gcc_wrappers/decl/base.h"
#include "gcc_wrappers/decl/function.h"
#include "gcc_wrappers/type/base.h"

#include "gcc_wrappers/_node_boilerplate-impl.define.h"

namespace gcc_wrappers {
   GCC_NODE_WRAPPER_BOILERPLATE(scope)
   
   /*static*/ bool scope::raw_node_is(tree t) {
      if (t == NULL_TREE)
         return false;
      
      // Based on the switch-case in `decl_type_context` in `tree.cc`:
      switch (TREE_CODE(t)) {
         case TRANSLATION_UNIT_DECL:
         case NAMESPACE_DECL:
         
         case TYPE_DECL:
         
         // Types and declarations can be scoped to a block or a function; 
         // blocks themselves are scoped to functions.
         case FUNCTION_DECL:
         case BLOCK:
         
         // These are the DECL_CONTEXT of their member CONST_DECLs.
         case ENUMERAL_TYPE:
            return true;
         
         // The only types that can contain other types or declarations.
         case RECORD_TYPE:
         case UNION_TYPE:
         case QUAL_UNION_TYPE:
            return true;
            
         default:
            break;
      }
      
      return false;
   }
   
   optional_scope scope::containing_scope() {
      return ::get_containing_scope(this->_node);
   }
   
   bool scope::is_declaration() const {
      return decl::base::raw_node_is(this->_node);
   }
   bool scope::is_file_scope() const {
      return TREE_CODE(this->_node) == TRANSLATION_UNIT_DECL;
   }
   bool scope::is_namespace() const {
      return TREE_CODE(this->_node) == NAMESPACE_DECL;
   }
   bool scope::is_type() const {
      return type::base::raw_node_is(this->_node);
   }
   
   decl::base scope::as_declaration() {
      assert(is_declaration());
      return decl::base::wrap(this->_node);
   }
   type::base scope::as_type() {
      assert(is_type());
      return type::base::wrap(this->_node);
   }
         
   decl::optional_function scope::nearest_function() {
      // Can't use this; it assumes the argument is a DECL.
      //return decl_function_context(this->_node);
      
      optional_scope current = *this;
      while (current && current->code() != FUNCTION_DECL) {
         current = current->containing_scope();
      }
      return current.unwrap();
   }
   
   type::optional_base scope::nearest_type() {
      optional_scope current = *this;
      while (current) {
         switch (current->code()) {
            case TRANSLATION_UNIT_DECL:
            case NAMESPACE_DECL:
               return {};
            case ENUMERAL_TYPE:
            case RECORD_TYPE:
            case UNION_TYPE:
            case QUAL_UNION_TYPE:
               return current.unwrap();
            case TYPE_DECL:
            case FUNCTION_DECL:
            case BLOCK:
               break;
            default:
               assert(false && "unreachable");
         }
         current = current->containing_scope();
      }
      return {};
   }
}