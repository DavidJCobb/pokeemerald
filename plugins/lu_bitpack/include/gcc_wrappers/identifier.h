#pragma once
#include <string_view>
#include "lu/strings/zview.h"
#include "gcc_wrappers/node.h"
#include "gcc_wrappers/optional.h"
#include "gcc_wrappers/_node_boilerplate.define.h"

namespace gcc_wrappers {
   class identifier : public node {
      public:
         static bool raw_node_is(tree t);
         GCC_NODE_WRAPPER_BOILERPLATE(identifier)
         
      public:
         identifier(lu::strings::zview);
         
         lu::strings::zview name() const;
   };
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(identifier);
}

#include "gcc_wrappers/_node_boilerplate.undef.h"