#pragma once
#include <string_view>
#include "gcc_wrappers/node.h"
#include "gcc_wrappers/optional.h"
#include "gcc_wrappers/attribute_list.h"
#include "gcc_wrappers/scope.h"
#include "gcc_wrappers/_node_boilerplate.define.h"

namespace gcc_wrappers {
   namespace decl {
      class base : public node {
         public:
            static bool raw_node_is(tree t) {
               return DECL_P(t);
            }
            GCC_NODE_WRAPPER_BOILERPLATE(base)
            
         public:
            std::string_view name() const;
            
            std::string fully_qualified_name() const;
            
            attribute_list attributes();
            const attribute_list attributes() const;
            
            // If the declaration appears in multiple places (e.g. a 
            // forward-declared entity that is later defined), this 
            // refers to the definition.
            location_t source_location() const;
            
            std::string_view source_file() const;
            int source_line() const;
            
            bool is_at_file_scope() const;
            
            bool is_artificial() const;
            void make_artificial();
            void set_is_artificial(bool);
            
            bool is_sym_debugger_ignored() const; // DECL_IGNORED_P
            void make_sym_debugger_ignored();
            void set_is_sym_debugger_ignored(bool);
            
            bool is_used() const;
            void make_used();
            void set_is_used(bool);
            
            optional_scope context() const;
      };
      DECLARE_GCC_OPTIONAL_NODE_WRAPPER(base);
   }
}

#include "gcc_wrappers/_node_boilerplate.undef.h"