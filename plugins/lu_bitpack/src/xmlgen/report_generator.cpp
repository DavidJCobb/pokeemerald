#include "xmlgen/report_generator.h"
#include <cassert>
#include <limits>
#include "lu/stringf.h"
#include "bitpacking/data_options.h"
#include "bitpacking/global_options.h"
#include "basic_global_state.h"
#include "codegen/decl_descriptor.h"
#include "codegen/decl_dictionary.h"
#include "codegen/generation_request.h"
#include "codegen/stats_gatherer.h"
#include "codegen/whole_struct_function_dictionary.h"
#include "codegen/whole_struct_function_info.h"
#include "gcc_wrappers/constant/floating_point.h"
#include "gcc_wrappers/constant/integer.h"
#include "gcc_wrappers/constant/string.h"
#include "gcc_wrappers/decl/param.h"
#include "gcc_wrappers/decl/type_def.h"
#include "gcc_wrappers/type/array.h"
#include "gcc_wrappers/type/record.h"
#include "gcc_wrappers/type/untagged_union.h"
#include "gcc_wrappers/identifier.h"
#include <c-family/c-common.h> // lookup_name
namespace gw {
   using namespace gcc_wrappers;
}
namespace typed_options {
   using namespace bitpacking::typed_data_options::computed;
}

#include "codegen/instructions/base.h"
#include "codegen/instructions/array_slice.h"
#include "codegen/instructions/padding.h"
#include "codegen/instructions/single.h"
#include "codegen/instructions/transform.h"
#include "codegen/instructions/union_switch.h"
#include "codegen/instructions/utils/walk.h"

#include "xmlgen/bitpacking_default_value_to_xml.h"
#include "xmlgen/bitpacking_x_options_to_xml.h"
#include "xmlgen/referenceable_struct_members_to_xml.h"

namespace {
   using owned_element = xmlgen::report_generator::owned_element;
}

namespace xmlgen {
   report_generator::category_info& report_generator::_get_or_create_category_info(std::string_view name) {
      for(auto& info : this->_categories)
         if (info.name == name)
            return info;
      auto& info = this->_categories.emplace_back();
      info.name = name;
      return info;
   }
   report_generator::type_info& report_generator::_get_or_create_type_info(gw::type::base type) {
      for(auto& info : this->_types)
         if (info.stats.type.unwrap() == type.unwrap())
            return info;
      auto& info = this->_types.emplace_back(type);
      return info;
   }
   
   std::string report_generator::_loop_variable_to_string(codegen::decl_descriptor_pair pair) const {
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
   std::string report_generator::_variable_to_string(codegen::decl_descriptor_pair pair) const {
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
   std::string report_generator::_value_path_to_string(const codegen::value_path& path) const {
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
   
   owned_element report_generator::_generate(const codegen::instructions::array_slice& instr) {
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
   owned_element report_generator::_generate(const codegen::instructions::padding& instr) {
      auto  node_ptr = std::make_unique<xml_element>();
      auto& node     = *node_ptr;
      
      node.node_name = "padding";
      node.set_attribute_i("bitcount", instr.bitcount);
      
      return node_ptr;
   }
   owned_element report_generator::_generate(const codegen::instructions::single& instr) {
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
      bitpacking_default_value_to_xml(node, options);
      bitpacking_x_options_to_xml(node, options, false);
      
      return node_ptr;
   }
   owned_element report_generator::_generate(const codegen::instructions::transform& instr) {
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
   owned_element report_generator::_generate(const codegen::instructions::union_switch& instr) {
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
   
   owned_element report_generator::_generate(const codegen::instructions::base& instr) {
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
   
   owned_element report_generator::_generate_root(const codegen::instructions::container& root) {
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
   
   void report_generator::_sort_category_list() {
      auto& list = this->_categories;
      std::sort(
         list.begin(),
         list.end(),
         [](const auto& info_a, const auto& info_b) {
            return info_a.name.compare(info_b.name) < 0;
         }
      );
   }
   void report_generator::_sort_type_list() {
      auto& list = this->_types;
      std::sort(
         list.begin(),
         list.end(),
         [](const auto& info_a, const auto& info_b) {
            auto type_a = info_a.stats.type;
            auto type_b = info_b.stats.type;
            
            bool a_is_tag = true;
            bool b_is_tag = true;
            auto name_a = type_a.tag_name();
            auto name_b = type_b.tag_name();
            if (name_a.empty()) {
               auto decl = type_a.declaration();
               if (decl) {
                  name_a = decl->name();
                  a_is_tag = false;
               }
            }
            if (name_b.empty()) {
               auto decl = type_b.declaration();
               if (decl) {
                  name_b = decl->name();
                  b_is_tag = false;
               }
            }
            //
            // Compare the names according to the following criteria, in 
            // order of descending priority:
            //
            //  - ASCII-case-insensitive comparison
            //  - Length comparison (shorter first)
            //  - ASCII-case-sensitive comparison (uppercase first)
            //  - Sort tag names before symbol names
            //  - Sort by GCC type-node pointer (last-resort fallback)
            //
            size_t end = name_a.size();
            if (end > name_b.size())
               end = name_b.size();
            
            for(size_t i = 0; i < end; ++i) { // case-insensitive
               char c = name_a[i];
               char d = name_b[i];
               if (c >= 'a' && c <= 'z')
                  c -= 0x20;
               if (d >= 'a' && d <= 'z')
                  d -= 0x20;
               if (c == d)
                  continue;
               return (c < d);
            }
            {
               size_t size_a = name_a.size();
               size_t size_b = name_b.size();
               if (size_a < size_b)
                  return true;
               if (size_a > size_b)
                  return false;
            }
            for(size_t i = 0; i < end; ++i) {
               char c = name_a[i];
               char d = name_b[i];
               if (c == d)
                  continue;
               return (c & 0x20) == 0; // whether a is uppercase
            }
            if (a_is_tag != b_is_tag) {
               return a_is_tag; // tag names before symbol names
            }
            return type_a.unwrap() < type_b.unwrap(); // fallback
         }
      );
   }
   
   void report_generator::process(const codegen::generation_request& request) {
      auto& dict = codegen::decl_dictionary::get();
      for(size_t i = 0; i < request.identifier_groups.size(); ++i) {
         const auto& group = request.identifier_groups[i];
         for(size_t j = 0; j < group.size(); ++j) {
            const auto& item = group[j];
            
            auto& dst = this->_top_level_identifiers.emplace_back();
            dst.identifier        = item.id.name();
            dst.dereference_count = item.dereference_count;
            if (i > 0 && j == 0)
               dst.force_to_next_sector = true;
            
            auto decl = gw::decl::variable::wrap(lookup_name((tree)item.id.unwrap()));
            dst.original_type = decl.value_type();
            const auto& desc =
               item.dereference_count > 0 ?
                  dict.dereference_and_describe(decl, item.dereference_count)
               :
                  dict.describe(decl)
            ;
            
            dst.serialized_type = desc.types.serialized;
            dst.options         = desc.options;
            
            if (dst.original_type->is_integral())
               this->_integral_types.index_type(dst.original_type->as_integral());
            if (dst.serialized_type->is_integral())
               this->_integral_types.index_type(dst.serialized_type->as_integral());
         }
      }
   }
   void report_generator::process(const codegen::instructions::container& root) {
      auto& info = this->_sectors.emplace_back();
      info.instructions = _generate_root(root);
   }
   void report_generator::process(const codegen::whole_struct_function_dictionary& dict) {
      dict.for_each([this](gw::type::base type, const codegen::whole_struct_function_info& ws_info) {
         auto& info = this->_get_or_create_type_info(type);
         info.instructions = _generate_root(*ws_info.instructions_root->as<codegen::instructions::container>());
         
         if (type.is_container())
            this->_integral_types.index_types_in(type.as_container());
      });
   }
   void report_generator::process(const codegen::stats_gatherer& gatherer) {
      {  // Per-sector stats
         size_t end = (std::min)(gatherer.sectors.size(), this->_sectors.size());
         for(size_t i = 0; i < end; ++i) {
            auto& src  = gatherer.sectors[i];
            auto& info = this->_sectors[i];
            info.stats = src;
         }
      }
      
      // Per-type stats
      for(const auto& pair : gatherer.types) {
         auto         type  = pair.first;
         const auto&  stats = pair.second;
         
         auto& info = this->_get_or_create_type_info(type);
         info.stats = stats;
         if (type.is_container())
            this->_integral_types.index_types_in(type.as_container());
      }
      
      // Category stats.
      for(auto& pair : gatherer.categories) {
         const auto& name  = pair.first;
         const auto& stats = pair.second;
         
         auto& info = this->_get_or_create_category_info(name);
         info.stats = stats;
      }
      
      // Done with gatherer.
   }
   
   std::string report_generator::bake() {
      std::string out = "<data>\n";
      {  // global options
         const auto& gs = basic_global_state::get();
         const auto& go = gs.global_options;
         
         out += "   <config>\n";
         {
            auto  node_ptr = std::make_unique<xml_element>();
            auto& node     = *node_ptr;
            node.node_name = "option";
            node.set_attribute("name", "max-sector-count");
            node.set_attribute_i("value", go.sectors.max_count);
            out += node.to_string(2);
            out += '\n';
         }
         if (go.sectors.bytes_per != std::numeric_limits<size_t>::max()) {
            auto  node_ptr = std::make_unique<xml_element>();
            auto& node     = *node_ptr;
            node.node_name = "option";
            node.set_attribute("name", "max-sector-bytecount");
            node.set_attribute_i("value", go.sectors.bytes_per);
            out += node.to_string(2);
            out += '\n';
         }
         out += "   </config>\n";
      }
      {  // categories
         auto& list = this->_categories;
         if (!list.empty()) {
            this->_sort_category_list();
            out += "   <categories>\n";
            for(const auto& info : list) {
               auto node_ptr = info.stats.to_xml(info.name);
               out += node_ptr->to_string(2);
               out += '\n';
            }
            out += "   </categories>\n";
         }
      }
      {  // seen types
         out += "   <c-types>\n";
         {  // integral types
            this->_integral_types.sort_all();
            this->_integral_types.for_each_canonical_info([&out](auto& item) {
               out += item.to_xml()->to_string(2);
               out += '\n';
            });
         }
         {  // struct/union types
            auto& list = this->_types;
            this->_sort_type_list();
            for(auto& info : list) {
               auto  node_ptr = info.stats.to_xml();
               {
                  bitpacking::data_options options;
                  options.load(info.stats.type);
                  //
                  bitpacking_x_options_to_xml(
                     *node_ptr,
                     options,
                     true
                  );
               }
               if (info.stats.type.is_container()) {
                  auto cont      = info.stats.type.as_container();
                  auto child_ptr = referenceable_struct_members_to_xml(cont, &this->_integral_types);
                  node_ptr->append_child(std::move(child_ptr));
               }
               
               xml_element* instr = nullptr;
               if (info.instructions) {
                  instr = info.instructions.get();
                  node_ptr->append_child(std::move(info.instructions));
               }
               out += node_ptr->to_string(2);
               out += '\n';
               if (instr) {
                  info.instructions = node_ptr->take_child(*instr);
               }
            }
         }
         out += "   </c-types>\n";
      }
      {  // top-level values to save
         out += "   <top-level-values>\n";
         for(size_t i = 0; i < this->_top_level_identifiers.size(); ++i) {
            auto& ident = this->_top_level_identifiers[i];
            auto  node_ptr = std::make_unique<xml_element>();
            auto& node     = *node_ptr;
            node.set_attribute("name", ident.identifier);
            if (ident.dereference_count > 0)
               node.set_attribute_i("dereference-count", ident.dereference_count);
            if (ident.force_to_next_sector)
               node.set_attribute_b("force-to-next-sector", true);
            
            if (ident.original_type) {
               auto pp = ident.original_type->pretty_print();
               if (pp != "<unnamed>")
                  node.set_attribute("type", pp);
            }
            if (ident.serialized_type) {
               auto pp = ident.serialized_type->pretty_print();
               if (pp != "<unnamed>")
                  node.set_attribute("serialized-type", pp);
            }
            
            bitpacking_x_options_to_xml(node, ident.options, false);
            bitpacking_default_value_to_xml(node, ident.options);
            //
            // Categories:
            //
            for(const auto& category : ident.options.stat_categories) {
               auto  elem_ptr = std::make_unique<xml_element>();
               auto& elem     = *elem_ptr;
               elem.node_name = "category";
               elem.set_attribute("name", category);
               node.append_child(std::move(elem_ptr));
            }
            for(const auto& text : ident.options.misc_annotations) {
               auto  elem_ptr = std::make_unique<xml_element>();
               auto& elem     = *elem_ptr;
               elem.node_name = "annotation";
               elem.set_attribute("text", text);
               node.append_child(std::move(elem_ptr));
            }
            
            out += node.to_string(2);
            out += '\n';
         }
         out += "   </top-level-values>\n";
      }
      {  // sectors
         auto& list = this->_sectors;
         if (!list.empty()) {
            out += "   <sectors>\n";
            for(auto& info : list) {
               auto  node_ptr = std::make_unique<xml_element>();
               node_ptr->node_name = "sector";
               node_ptr->append_child(info.stats.to_xml());
               
               xml_element* instr = nullptr;
               if (info.instructions) {
                  instr = info.instructions.get();
                  node_ptr->append_child(std::move(info.instructions));
               }
               out += node_ptr->to_string(2);
               out += '\n';
               if (instr) {
                  info.instructions = node_ptr->take_child(*instr);
               }
            }
            out += "   </sectors>\n";
         }
      }
      out += "</data>";
      return out;
   }
}