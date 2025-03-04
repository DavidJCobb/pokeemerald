#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace xmlgen {
   class xml_element {
      public:
         using attribute = std::pair<std::string, std::string>; // name, value
         
         static constexpr const size_t index_of_none = std::string::npos;
      
      public:
         std::string  node_name;
         std::vector<attribute> attributes;
         std::vector<std::unique_ptr<xml_element>> children;
         xml_element* parent = nullptr;
         std::string  text_content;
         
      protected:
         size_t _index_of_attribute(std::string_view) const noexcept;
         
      public:
         void append_child(std::unique_ptr<xml_element>&&);
         void remove_child(xml_element&); // deletes the child
         std::unique_ptr<xml_element> take_child(xml_element&);
         
         xml_element* first_child_by_node_name(std::string_view);
         const xml_element* first_child_by_node_name(std::string_view) const;
         
         void remove_attribute(std::string_view name);
         
         // Asserts that the name is a valid name.
         void set_attribute(std::string_view name, std::string_view value);
         
         // Convenience functions which stringify the value before calling the 
         // main overload.
         void set_attribute_b(std::string_view name, bool value);
         void set_attribute_i(std::string_view name, intmax_t value);
         void set_attribute_f(std::string_view name, float value);
         
         std::string to_string(size_t indent_level = 0) const;
   };
}