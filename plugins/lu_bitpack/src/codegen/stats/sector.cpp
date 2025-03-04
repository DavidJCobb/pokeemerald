#include "codegen/stats/sector.h"

using xml_element = xmlgen::xml_element;

namespace codegen::stats {
   std::unique_ptr<xml_element> sector::to_xml() const {
      auto  node_ptr = std::make_unique<xml_element>();
      auto& node     = *node_ptr;
      node.node_name = "stats";
      
      if (this->total_packed_size) {
         auto  child_ptr = std::make_unique<xml_element>();
         auto& child     = *child_ptr;
         child.node_name = "bitcounts";
         node.append_child(std::move(child_ptr));
         
         child.set_attribute_i("total-packed", this->total_packed_size);
      }
      
      return node_ptr;
   }
}