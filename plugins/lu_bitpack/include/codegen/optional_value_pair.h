#pragma once
#include "lu/strings/zview.h"
#include "gcc_wrappers/value.h"

namespace codegen {
   struct decl_descriptor_pair;
}

namespace codegen {
   struct optional_value_pair {
      optional_value_pair() {}
      optional_value_pair(const decl_descriptor_pair&);
      optional_value_pair(gcc_wrappers::value read, gcc_wrappers::value save);
      optional_value_pair(gcc_wrappers::optional_value read, gcc_wrappers::optional_value save);
      
      gcc_wrappers::optional_value read;
      gcc_wrappers::optional_value save;
      
      optional_value_pair access_array_element(const optional_value_pair&) const;
      optional_value_pair access_member(const char* name) const;
      optional_value_pair dereference() const;
   };
}