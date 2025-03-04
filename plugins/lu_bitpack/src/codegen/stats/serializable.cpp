#include "codegen/stats/serializable.h"

using xml_element = xmlgen::xml_element;

namespace codegen::stats {
   bool serializable::empty() const {
      if (this->counts.total > 0)
         return false;
      if (this->bitcounts.total_packed > 0)
         return false;
      if (this->bitcounts.total_unpacked > 0)
         return false;
      if (!this->counts.by_sector.empty())
         return false;
      if (!this->counts.by_top_level_identifier.empty())
         return false;
      return true;
   }
   
   std::unique_ptr<xml_element> serializable::to_xml() const {
      auto  node_ptr = std::make_unique<xml_element>();
      auto& node     = *node_ptr;
      node.node_name = "stats";
      
      {
         auto  child_ptr = std::make_unique<xml_element>();
         auto& child     = *child_ptr;
         child.node_name = "bitcounts";
         node.append_child(std::move(child_ptr));
         child.set_attribute_i("total-unpacked", this->bitcounts.total_unpacked);
         child.set_attribute_i("total-packed",   this->bitcounts.total_packed);
      }
      if (this->counts.total > 0 || !this->counts.by_sector.empty() || !this->counts.by_top_level_identifier.empty()) {
         auto  child_ptr = std::make_unique<xml_element>();
         auto& child     = *child_ptr;
         child.node_name = "counts";
         node.append_child(std::move(child_ptr));
         
         if (this->counts.total > 0)
            child.set_attribute_i("total", this->counts.total);
         
         if (const auto& list = this->counts.by_sector; !list.empty()) {
            for(const auto& item : list) {
               auto  grandchild_ptr = std::make_unique<xml_element>();
               auto& grandchild     = *grandchild_ptr;
               grandchild.node_name = "in-sector";
               child.append_child(std::move(grandchild_ptr));
               
               grandchild.set_attribute_i("index", item.sector_index);
               grandchild.set_attribute_i("count", item.count);
            }
         }
         if (const auto& list = this->counts.by_top_level_identifier; !list.empty()) {
            for(const auto& item : list) {
               auto  grandchild_ptr = std::make_unique<xml_element>();
               auto& grandchild     = *grandchild_ptr;
               grandchild.node_name = "in-top-level-value";
               child.append_child(std::move(grandchild_ptr));
               
               grandchild.set_attribute("name",  item.identifier);
               grandchild.set_attribute_i("count", item.count);
            }
         }
      }
      
      return node_ptr;
   }
   
   //
   // Modifiers:
   //
   
   void serializable::add_count_in_sector(size_t sector_index, size_t count) {
      auto& list = this->counts.by_sector;
      for(auto& info : list) {
         if (info.sector_index == sector_index) {
            info.count += count;
            return;
         }
      }
      auto& info = list.emplace_back();
      info.sector_index = sector_index;
      info.count        = count;
   }
   void serializable::add_count_in_top_level_identifier(std::string_view identifier, size_t count) {
      auto& list = this->counts.by_top_level_identifier;
      for(auto& info : list) {
         if (info.identifier == identifier) {
            info.count += count;
            return;
         }
      }
      auto& info = list.emplace_back();
      info.identifier = identifier;
      info.count      = count;
   }
}