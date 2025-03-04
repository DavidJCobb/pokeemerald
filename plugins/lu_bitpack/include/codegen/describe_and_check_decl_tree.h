#pragma once
#include "gcc_wrappers/decl/variable.h"

namespace codegen {
   class decl_descriptor;
}

namespace codegen {
   // Create `decl_descriptor`s for a variable and all DECLs therein (i.e. 
   // struct members; members of members; et cetera). Returns true if all 
   // DECLs compute bitpacking options without any errors; else false.
   extern bool describe_and_check_decl_tree(gcc_wrappers::decl::variable);
   
   extern bool describe_and_check_decl_tree(const decl_descriptor&);
}