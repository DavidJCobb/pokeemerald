#pragma once
#include <memory>
#include "xmlgen/xml_element.h"

namespace bitpacking {
   class data_options;
}

namespace xmlgen {
   extern void bitpacking_default_value_to_xml(
      xml_element& subject,
      const bitpacking::data_options& options
   );
}