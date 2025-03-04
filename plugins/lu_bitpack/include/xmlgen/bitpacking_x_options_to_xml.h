#pragma once
#include <memory>
#include "xmlgen/xml_element.h"

namespace bitpacking {
   class data_options;
}

namespace xmlgen {
   extern void bitpacking_x_options_to_xml(
      xml_element& subject,
      const bitpacking::data_options& options,
      
      // TRUE: Create a new child node whose name depends on the option type,
      //       and write attributes to it.
      //
      // FALSE: Change the node name and attributes of `subject`.
      bool write_as_child_node
   );
}