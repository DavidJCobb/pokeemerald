#pragma once
#include <memory>
#include "gcc_wrappers/type/container.h"
#include "xmlgen/xml_element.h"

namespace codegen {
   class decl_dictionary;
}

namespace xmlgen {
   extern std::unique_ptr<xml_element> referenceable_struct_members_to_xml(
      gcc_wrappers::type::container
   );
}