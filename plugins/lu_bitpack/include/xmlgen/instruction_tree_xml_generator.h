#pragma once
#include <memory>
#include <string>
#include <vector>
#include "codegen/decl_descriptor_pair.h"
#include "xmlgen/xml_element.h"
namespace bitpacking {
   class data_options;
}
namespace codegen {
   namespace instructions {
      class base;
      class container;
      
      class array_slice;
      class padding;
      class single;
      class transform;
      class union_switch;
   }
   class value_path;
}

namespace xmlgen {
   class instruction_tree_xml_generator {
      public:
         using owned_element = std::unique_ptr<xml_element>;
      
      protected:
         // Used to give each loop variable a unique name and ID. We gather 
         // these up before generating XML. Each variable's ID is its index 
         // in this list.
         std::vector<codegen::decl_descriptor_pair> _loop_variables;
         
         // Similarly, used to give each transformed value a unique name.
         std::vector<codegen::decl_descriptor_pair> _transformed_values;
         
      protected:
         std::string _loop_variable_to_string(codegen::decl_descriptor_pair) const;
         std::string _variable_to_string(codegen::decl_descriptor_pair) const;
         std::string _value_path_to_string(const codegen::value_path&) const;
         
         void _fill_out_value_element(xml_element&, const bitpacking::data_options&);
         
         owned_element _generate(const codegen::instructions::array_slice&);
         owned_element _generate(const codegen::instructions::padding&);
         owned_element _generate(const codegen::instructions::single&);
         owned_element _generate(const codegen::instructions::transform&);
         owned_element _generate(const codegen::instructions::union_switch&);
         
         owned_element _generate(const codegen::instructions::base&);
         
      public:
         // Call on a tree root.
         owned_element generate(const codegen::instructions::container&);
   };
}