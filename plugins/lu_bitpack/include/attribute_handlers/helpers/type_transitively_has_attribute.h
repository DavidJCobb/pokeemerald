#pragma once
#include "lu/strings/zview.h"
#include "gcc_wrappers/type/base.h"

namespace attribute_handlers::helpers {
   //
   // Returns true for any of the following:
   //
   //  - The type has the given attribute.
   //
   //  - The type is a typedef [of a typedef [...]], and any of the 
   //    type(def)s upward in the chain have the given attribute.
   //
   //  - The type is an array [of an array [...]], and any of the 
   //    inner types have the given attribute.
   //
   extern bool type_transitively_has_attribute(gcc_wrappers::type::base, lu::strings::zview name);
}