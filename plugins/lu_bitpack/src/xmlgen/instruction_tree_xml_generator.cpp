#include "xmlgen/instruction_tree_xml_generator.h"
#include <cassert>
#include <limits>
#include "gcc_wrappers/decl/param.h"
#include "bitpacking/data_options.h"
#include "lu/stringf.h"
#include "codegen/decl_descriptor.h"
#include "codegen/decl_dictionary.h"
#include "codegen/value_path.h"
#include "xmlgen/bitpacking_default_value_to_xml.h"
namespace gw {
   using namespace gcc_wrappers;
}

#include "codegen/instructions/base.h"
#include "codegen/instructions/array_slice.h"
#include "codegen/instructions/padding.h"
#include "codegen/instructions/single.h"
#include "codegen/instructions/transform.h"
#include "codegen/instructions/union_switch.h"
#include "codegen/instructions/utils/walk.h"

namespace typed_options {
   using namespace bitpacking::typed_data_options::computed;
}
namespace {
   using owned_element = xmlgen::instruction_tree_xml_generator::owned_element;
}

namespace xmlgen {
   std::string instruction_tree_xml_generator::_loop_variable_to_string(codegen::decl_descriptor_pair pair) const {
      const auto& list = this->_loop_variables;
      if (list.size() <= 26) {
         for(size_t i = 0; i < list.size(); ++i) {
            if (list[i] != pair)
               continue;
            std::string name = "__a";
            name[2] += i;
            return name;
         }
      } else {
         for(size_t i = 0; i < list.size(); ++i)
            if (list[i] == pair)
               return lu::stringf("__loop_var_%u", (int)i);
      }
      return "__unknown_var";
   }
   std::string instruction_tree_xml_generator::_variable_to_string(codegen::decl_descriptor_pair pair) const {
      for(size_t i = 0; i < this->_transformed_values.size(); ++i)
         if (pair == this->_transformed_values[i])
            return lu::stringf("__transformed_var_%u", (int)i);
         
      size_t deref = pair.read->variable.dereference_count;
      if (deref) {
         std::string out = "(";
         for(size_t i = 0; i < deref; ++i)
            out += '*';
         out += pair.read->decl.name();
         out += ')';
         return out;
      }
      
      return std::string(pair.read->decl.name());
   }
   std::string instruction_tree_xml_generator::_value_path_to_string(const codegen::value_path& path) const {
      std::string out;
      bool first = true;
      for(size_t i = 0; i < path.segments.size(); ++i) {
         const auto& segm = path.segments[i];
         const auto  pair = segm.descriptor();
         
         if (i == 0) {
            assert(!segm.is_array_access());
            auto decl = pair.read->decl;
            if (decl.is<gw::decl::param>()) {
               //
               // This is a whole-struct serialization function. Do not emit 
               // the name of the struct pointer parameter.
               //
               continue;
            }
         }
         
         if (segm.is_array_access()) {
            if (std::holds_alternative<size_t>(segm.data)) {
               out += lu::stringf("[%u]", (int)std::get<size_t>(segm.data));
               continue;
            }
            out += '[';
            out += this->_loop_variable_to_string(pair);
            out += ']';
            continue;
         }
         
         if (!first)
            out += '.';
         first = false;
         out += this->_variable_to_string(pair);
      }
      return out;
   }
   
   void instruction_tree_xml_generator::_fill_out_value_element(
      xml_element& node,
      const bitpacking::data_options& options
   ) {
      //
      // Set the node name.
      //
      if (options.is_omitted) {
         node.node_name = "omitted";
      } else {
         node.node_name = "unknown";
         if (options.is<typed_options::boolean>()) {
            node.node_name = "boolean";
         } else if (options.is<typed_options::buffer>()) {
            node.node_name = "buffer";
         } else if (options.is<typed_options::integral>()) {
            node.node_name = "integer";
         } else if (options.is<typed_options::pointer>()) {
            node.node_name = "pointer";
         } else if (options.is<typed_options::string>()) {
            node.node_name = "string";
         } else if (options.is<typed_options::structure>()) {
            node.node_name = "struct";
         } else if (options.is<typed_options::tagged_union>()) {
            auto& casted = options.as<typed_options::tagged_union>();
            if (casted.is_internal)
               node.node_name = "union-internal-tag";
            else
               node.node_name = "union-external-tag";
         } else if (options.is<typed_options::transformed>()) {
            node.node_name = "transformed";
         } else {
            node.node_name = "unknown";
         }
      }
      //
      // Set the attributes.
      //
      if (options.is<typed_options::buffer>()) {
         const auto& casted = options.as<typed_options::buffer>();
         node.set_attribute_i("bytecount", casted.bytecount);
      } else if (options.is<typed_options::integral>()) {
         const auto& casted = options.as<typed_options::integral>();
         node.set_attribute_i("bitcount", casted.bitcount);
         if (casted.min != typed_options::integral::no_minimum) {
            node.set_attribute_i("min", casted.min);
         }
         if (casted.max != typed_options::integral::no_maximum) {
            node.set_attribute_i("max", casted.max);
         }
      } else if (options.is<typed_options::pointer>()) {
         ;
      } else if (options.is<typed_options::string>()) {
         const auto& casted = options.as<typed_options::string>();
         node.set_attribute_i("length",    casted.length);
         node.set_attribute_b("nonstring", casted.nonstring);
      } else if (options.is<typed_options::structure>()) {
         ;
      } else if (options.is<typed_options::tagged_union>()) {
         const auto& casted = options.as<typed_options::tagged_union>();
         node.set_attribute("tag", casted.tag_identifier);
      } else if (options.is<typed_options::transformed>()) {
         const auto& casted = options.as<typed_options::transformed>();
         if (auto opt = casted.transformed_type)
            node.set_attribute("transformed-type", opt->pretty_print());
         if (auto opt = casted.pre_pack)
            node.set_attribute("pack-function",    opt->name());
         if (auto opt = casted.post_unpack)
            node.set_attribute("unpack-function",  opt->name());
      }
      //
      // Set the default value.
      //
      bitpacking_default_value_to_xml(node, options);
   }
   
   owned_element instruction_tree_xml_generator::_generate(const codegen::instructions::array_slice& instr) {
      auto  node_ptr = std::make_unique<xml_element>();
      auto& node     = *node_ptr;
      
      node.node_name = "loop";
      node.set_attribute("array", this->_value_path_to_string(instr.array.value));
      node.set_attribute_i("start", instr.array.start);
      node.set_attribute_i("count", instr.array.count);
      node.set_attribute("counter-var", this->_loop_variable_to_string(instr.loop_index.descriptors));
      
      for(auto& child_ptr : instr.instructions) {
         node.append_child(this->_generate(*child_ptr));
      }
      
      return node_ptr;
   }
   owned_element instruction_tree_xml_generator::_generate(const codegen::instructions::padding& instr) {
      auto  node_ptr = std::make_unique<xml_element>();
      auto& node     = *node_ptr;
      
      node.node_name = "padding";
      node.set_attribute_i("bitcount", instr.bitcount);
      
      return node_ptr;
   }
   owned_element instruction_tree_xml_generator::_generate(const codegen::instructions::single& instr) {
      auto  node_ptr = std::make_unique<xml_element>();
      auto& node     = *node_ptr;
      
      node.set_attribute("value", this->_value_path_to_string(instr.value));
      
      codegen::decl_descriptor_pair desc_pair;
      for(auto& segm : instr.value.segments) {
         if (segm.is_array_access())
            continue;
         desc_pair = segm.member_descriptor();
      }
      assert(desc_pair.read != nullptr);
      auto value_type = *desc_pair.read->types.serialized;
      node.set_attribute("type", value_type.pretty_print());
      
      const auto& options = instr.value.bitpacking_options();
      this->_fill_out_value_element(node, options);
      
      return node_ptr;
   }
   owned_element instruction_tree_xml_generator::_generate(const codegen::instructions::transform& instr) {
      auto  node_ptr = std::make_unique<xml_element>();
      auto& node     = *node_ptr;
      
      node.node_name = "transform";
      node.set_attribute("value", this->_value_path_to_string(instr.to_be_transformed_value));
      node.set_attribute("transformed-type", instr.types.back().name());
      node.set_attribute("transformed-value", this->_variable_to_string(instr.transformed.descriptors));
      if (instr.types.size() > 1) {
         std::string through;
         for(size_t i = 0; i < instr.types.size() - 1; ++i) {
            if (i > 0)
               through += ' ';
            through += instr.types[i].name();
         }
         node.set_attribute("transform-through", through);
      }
      
      for(auto& child_ptr : instr.instructions) {
         node.append_child(this->_generate(*child_ptr));
      }
      
      return node_ptr;
   }
   owned_element instruction_tree_xml_generator::_generate(const codegen::instructions::union_switch& instr) {
      auto  node_ptr = std::make_unique<xml_element>();
      auto& node     = *node_ptr;
      
      node.node_name = "switch";
      node.set_attribute("operand", this->_value_path_to_string(instr.condition_operand));
      
      for(const auto& pair : instr.cases) {
         auto  child_ptr = std::make_unique<xml_element>();
         auto& child     = *child_ptr;
         child.node_name = "case";
         child.set_attribute_i("value", pair.first);
         node.append_child(std::move(child_ptr));
         
         for(auto& nested : pair.second->instructions) {
            child.append_child(this->_generate(*nested));
         }
      }
      if (instr.else_case) {
         auto  child_ptr = std::make_unique<xml_element>();
         auto& child     = *child_ptr;
         child.node_name = "fallback-case";
         node.append_child(std::move(child_ptr));
         
         for(auto& nested : instr.else_case->instructions) {
            child.append_child(this->_generate(*nested));
         }
      }
      
      return node_ptr;
   }
   
   owned_element instruction_tree_xml_generator::_generate(const codegen::instructions::base& instr) {
      if (auto* casted = instr.as<codegen::instructions::array_slice>())
         return this->_generate(*casted);
      if (auto* casted = instr.as<codegen::instructions::padding>())
         return this->_generate(*casted);
      if (auto* casted = instr.as<codegen::instructions::single>())
         return this->_generate(*casted);
      if (auto* casted = instr.as<codegen::instructions::transform>())
         return this->_generate(*casted);
      if (auto* casted = instr.as<codegen::instructions::union_switch>())
         return this->_generate(*casted);
      
      assert(false && "unreachable");
   }
   
   owned_element instruction_tree_xml_generator::generate(const codegen::instructions::container& root) {
      this->_loop_variables.clear();
      this->_transformed_values.clear();
      
      codegen::instructions::utils::walk(
         [this](const codegen::instructions::base& node) {
            if (auto* casted = node.as<codegen::instructions::array_slice>()) {
               this->_loop_variables.push_back(casted->loop_index.descriptors);
               return;
            }
            if (auto* casted = node.as<codegen::instructions::transform>()) {
               this->_transformed_values.push_back(casted->transformed.descriptors);
               return;
            }
         },
         root
      );
      
      auto  node_ptr = std::make_unique<xml_element>();
      auto& node     = *node_ptr;
      node.node_name = "instructions";
      for(auto& child_ptr : root.instructions) {
         node.append_child(_generate(*child_ptr));
      }
      return node_ptr;
   }
}