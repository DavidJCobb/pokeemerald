#include "codegen/stats/category.h"

using xml_element = xmlgen::xml_element;

namespace codegen::stats {
   std::unique_ptr<xml_element> category::to_xml(std::string_view category_name) const {
      auto out_ptr = serializable::to_xml();
      out_ptr->node_name = "category";
      out_ptr->set_attribute("name", category_name);
      return out_ptr;
   }
}