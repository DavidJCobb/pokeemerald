#pragma once
#include <string_view>
#include "lu/strings/zview.h"
#include "gcc_wrappers/constant/base.h"
#include "gcc_wrappers/type/base.h"
#include "gcc_wrappers/_node_boilerplate.define.h"

namespace gcc_wrappers::constant {
   class string : public base {
      public:
         static bool raw_node_is(tree t) {
            return TREE_CODE(t) == STRING_CST;
         }
         GCC_NODE_WRAPPER_BOILERPLATE(string)
         
      public:
         string(const std::string&);
         string(lu::strings::zview);
         string(const char*, size_t size_including_null);
         
         std::string_view value() const;
         
         size_t length() const; // number of characters; excludes null terminator
         size_t size() const; // number of characters; includes null terminator
         
         // Each STRING_CST can only have a single type, but can be referenced 
         // by multiple ADDR_EXPRs (i.e. multiple string literals can share 
         // the same data).
         bool referenced_by_string_literal() const;
         
         // Creates a new string literal. If this STRING_CST is already used 
         // by a literal of a different type, then we'll have to clone the 
         // STRING_CST.
         //
         // If the input type is empty, we default to `char` (and we assert 
         // in that case that the built-in type nodes are actually ready).
         ::gcc_wrappers::value to_string_literal();
         ::gcc_wrappers::value to_string_literal(type::base character_type);
   };
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(string);
}

#include "gcc_wrappers/_node_boilerplate.undef.h"