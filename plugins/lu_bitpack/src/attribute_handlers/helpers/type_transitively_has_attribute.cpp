#include "attribute_handlers/helpers/type_transitively_has_attribute.h"
#include "gcc_wrappers/decl/type_def.h"
#include "gcc_wrappers/type/array.h"
#include "gcc_wrappers/attribute.h"
namespace gw {
   using namespace gcc_wrappers;
}

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
   extern bool type_transitively_has_attribute(gw::type::base type, lu::strings::zview name) {
      if (type.attributes().has_attribute(name))
         return true;
      
      gw::type::optional_base next = type;
      do {
         if (next->attributes().has_attribute(name))
            return true;
         auto decl = next->declaration();
         if (!decl)
            break;
         if (decl->attributes().has_attribute(name))
            return true;
         next = decl->is_synonym_of();
      } while (next);
      
      if (type.is_array())
         return type_transitively_has_attribute(type.as_array().value_type(), name);
      
      return false;
   }
}