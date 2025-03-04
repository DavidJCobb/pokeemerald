#pragma once
#include <string>

namespace codegen::instructions {
   class base;
}

namespace codegen::instructions::utils {
   // for debugging
   class tree_stringifier {
      public:
         std::string data;
         size_t      depth = 0; // tree node nesting depth
         
      protected:
         void _append_indent();
      
         void _stringify_text(const ::codegen::instructions::base&);
         void _stringify_children(const ::codegen::instructions::base&);
         
      public:
         void stringify(const ::codegen::instructions::base&);
   };
}