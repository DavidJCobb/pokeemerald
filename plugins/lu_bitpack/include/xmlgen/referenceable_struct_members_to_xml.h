#pragma once
#include <memory>
#include "gcc_wrappers/type/container.h"
#include "xmlgen/xml_element.h"

namespace codegen {
   class decl_dictionary;
}
namespace xmlgen {
   class integral_type_index;
}

namespace xmlgen {
   extern std::unique_ptr<xml_element> referenceable_struct_members_to_xml(
      gcc_wrappers::type::container,
      
      // Optional. If provided, this is used to avoid writing redundant 
      // bitpacking option attributes for integral values.
      const integral_type_index* integral_types = nullptr
   );
}