#include "gcc_wrappers/node.h"
#include <string>
#include "lu/stringf.h"
#include <diagnostic.h>

namespace gcc_wrappers {
   namespace impl {
      void wrap_fail_on_null(tree raw) {
         if (raw != NULL_TREE) {
            return;
         }
         //
         // The `internal_error` function aborts for us, emitting a compiler stack trace when possible.
         //
         internal_error("fault in a compiler plug-in: failed to wrap a %<tree%> node (unexpected %<NULL_TREE%>)");
      }
      
      static std::string _report_bad_node_info(tree raw) {
         if (TREE_CODE(raw) == IDENTIFIER_NODE) {
            auto data = IDENTIFIER_POINTER(raw);
            if (data)
               return lu::stringf(" - identifier node content is %<%s%>\n", IDENTIFIER_POINTER(raw));
            return " - identifier has null data\n";
         }
         if (DECL_P(raw)) {
            auto name = DECL_NAME(raw);
            if (name == NULL_TREE) {
               return " - declaration name is irretrievable (no DECL_NAME)\n";
            }
            if (TREE_CODE(name) == IDENTIFIER_NODE) {
               auto data = IDENTIFIER_POINTER(name);
               if (data)
                  return lu::stringf(" - declaration name is %<%s%>\n", data);
               return " - declaration name is irretrievable (empty identifier node)\n";
            }
            return " - declaration name is irretrievable (DECL_NAME is not an IDENTIFIER_NODE)\n";
         }
         if (TYPE_P(raw)) {
            std::string facts;
            if (TREE_CODE(raw) == POINTER_TYPE) {
               raw = TREE_TYPE(raw);
               if (raw == NULL_TREE) {
                  return " - pointed-to type is irretrievable\n";
               }
               if (TREE_CODE(raw) == POINTER_TYPE) {
                  return " - pointed-to type is itself a pointer type\n";
               }
               facts = " - the following information (if any) refers to the pointed-to type\n";
            }
            auto name = TYPE_NAME(raw);
            if (name == NULL_TREE) {
               facts += " - type name is irretrievable (no TYPE_NAME)\n";
               return facts;
            }
            if (TREE_CODE(name) == TYPE_DECL) {
               facts += " - type is named by a TYPE_DECL node\n";
               name = DECL_NAME(name);
               if (name == NULL_TREE) {
                  facts += " - the TYPE_DECL node has no retrievable name\n";
                  return facts;
               }
            }
            if (TREE_CODE(name) == IDENTIFIER_NODE) {
               auto data = IDENTIFIER_POINTER(name);
               if (data)
                  facts += lu::stringf(" - type name is %<%s%>\n", data);
               else
                  facts += " - type name is irretrievable (empty identifier node)";
            }
            return facts;
         }
         return {};
      }
      
      [[noreturn]] void _wrap_report_wrong_type(tree raw) {
         assert(raw != NULL_TREE);
         auto facts = lu::stringf("fault in a compiler plug-in: failed to wrap a %<tree%> node (wrong type)\n - tree code is %<%s%>\n", get_tree_code_name(TREE_CODE(raw)));
         {
            auto more = _report_bad_node_info(raw);
            if (!more.empty())
               facts += std::move(more);
         }
         //
         // The `internal_error` function aborts for us, emitting a compiler stack trace when possible.
         //
         internal_error(facts.c_str());
      }
   }
   
   /*static*/ node node::wrap(tree n) {
      impl::wrap_fail_on_null(n);
      node out;
      out._node = n;
      return out;
   }
   
   tree_code node::code() const {
      return TREE_CODE(this->_node);
   }
   const std::string_view node::code_name() const noexcept {
      return get_tree_code_name(TREE_CODE(this->_node));
   }
}