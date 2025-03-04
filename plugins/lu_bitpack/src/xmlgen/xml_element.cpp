#include "xmlgen/xml_element.h"
#include <cassert>
#include <inttypes.h>
#include <utility> // std::as_const
#include "lu/stringf.h"
#include "xmlgen/is_valid_name.h"
#include "xmlgen/write_string_as_attribute_value.h"
#include "xmlgen/write_string_as_text_content.h"

namespace xmlgen {
   size_t xml_element::_index_of_attribute(std::string_view name) const noexcept {
      for(size_t i = 0; i < this->attributes.size(); ++i)
         if (this->attributes[i].first == name)
            return i;
      return index_of_none;
   }
   
   void xml_element::append_child(std::unique_ptr<xml_element>&& child) {
      assert(child != nullptr);
      if (child->parent) {
         assert(child->parent != this);
         child->parent->remove_child(*child);
      }
      child->parent = this;
      this->children.emplace_back() = std::move(child);
   }
   void xml_element::remove_child(xml_element& child) {
      assert(child.parent == this);
      auto&  list = this->children;
      size_t size = list.size();
      for(size_t i = 0; i < size; ++i) {
         if (list[i].get() == &child) {
            list.erase(list.begin() + i);
            return;
         }
      }
   }
   std::unique_ptr<xml_element> xml_element::take_child(xml_element& child) {
      assert(child.parent == this);
      
      std::unique_ptr<xml_element> result;
      
      auto&  list = this->children;
      size_t size = list.size();
      for(size_t i = 0; i < size; ++i) {
         if (list[i].get() == &child) {
            result = std::move(list[i]);
            list.erase(list.begin() + i);
            break;
         }
      }
      return result;
   }
   
   xml_element* xml_element::first_child_by_node_name(std::string_view name) {
      return const_cast<xml_element*>(std::as_const(*this).first_child_by_node_name(name));
   }
   const xml_element* xml_element::first_child_by_node_name(std::string_view name) const {
      for(const auto& node_ptr : this->children)
         if (node_ptr->node_name == name)
            return node_ptr.get();
      return nullptr;
   }
   
   void xml_element::remove_attribute(std::string_view name) {
      size_t i = this->_index_of_attribute(name);
      if (i == index_of_none)
         return;
      this->attributes.erase(this->attributes.begin() + i);
   }
   
   void xml_element::set_attribute(std::string_view name, std::string_view value) {
      assert(is_valid_name(name));
      size_t i = this->_index_of_attribute(name);
      if (i == index_of_none) {
         auto& attr = this->attributes.emplace_back();
         attr.first  = std::string(name);
         attr.second = value;
         return;
      }
      this->attributes[i].second = value;
   }
   void xml_element::set_attribute_b(std::string_view name, bool value) {
      this->set_attribute(name, value ? "true" : "false");
   }
   void xml_element::set_attribute_i(std::string_view name, intmax_t value) {
      auto vs = lu::stringf("%" PRIdMAX, value);
      this->set_attribute(name, vs);
   }
   void xml_element::set_attribute_f(std::string_view name, float value) {
      auto vs = lu::stringf("%f", value);
      this->set_attribute(name, vs);
   }
   
   std::string xml_element::to_string(size_t indent_level) const {
      std::string indent(indent_level * 3, ' ');
      std::string out;
      
      out += indent;
      out += '<';
      out += this->node_name;
      for(const auto& pair : this->attributes) {
         out += ' ';
         out += pair.first;
         out += '=';
         out += '"';
         write_string_as_attribute_value(pair.second, '"', out);
         out += '"';
      }
      if (this->children.empty() && this->text_content.empty()) {
         out += " />";
      } else {
         out += '>';
         
         const bool indent_contents = !this->children.empty();
         
         if (indent_contents) {
            out += '\n';
         }
         
         if (!this->text_content.empty()) {
            if (indent_contents) {
               out += indent;
               out += "   ";
            }
            write_string_as_text_content(this->text_content, out);
            if (indent_contents) {
               out += '\n';
               out += indent;
               out += "   ";
            }
         }
         for(const auto& child_ptr : this->children) {
            assert(child_ptr != nullptr);
            auto& child = *child_ptr.get();
            
            out += child.to_string(indent_level + 1);
            out += '\n';
         }
         
         if (indent_contents) {
            out += indent;
         }
         out += "</";
         out += this->node_name;
         out += '>';
      }
      
      return out;
   }
   
}