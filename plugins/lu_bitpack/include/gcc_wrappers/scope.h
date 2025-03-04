#pragma once
#include "gcc_wrappers/node.h"
#include "gcc_wrappers/optional.h"
#include "gcc_wrappers/_node_boilerplate.define.h"

namespace gcc_wrappers {
   namespace decl {
      class base;
      class function;
      DECLARE_GCC_OPTIONAL_NODE_WRAPPER(function);
   }
   namespace type {
      class base;
      DECLARE_GCC_OPTIONAL_NODE_WRAPPER(base);
   }
}

namespace gcc_wrappers {
   class scope;
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(scope);
   
   class scope : public node {
      public:
         static bool raw_node_is(tree t);
         GCC_NODE_WRAPPER_BOILERPLATE(scope)
         
      public:
         optional_scope containing_scope();
         
         bool is_declaration() const;
         bool is_file_scope() const;
         bool is_namespace() const;
         bool is_type() const;
         
         decl::base as_declaration();
         type::base as_type();
         
         // Returns self, the nearest enclosing function, or nullptr.
         decl::optional_function nearest_function();
         
         // Returns self, the nearest enclosing type, or nullptr.
         type::optional_base nearest_type();
   };
}

#include "gcc_wrappers/_node_boilerplate.undef.h"