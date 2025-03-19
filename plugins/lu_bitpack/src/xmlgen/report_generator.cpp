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

#include "xmlgen/bitpacking_default_value_to_xml.h"
#include "xmlgen/bitpacking_x_options_to_xml.h"
#include "xmlgen/instruction_tree_xml_generator.h"

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
   
   const bitpacking::data_options* report_generator::_get_serialized_type_options(gw::type::base type) {
      while (type.is_array())
         type = type.as_array().value_type();
      if (type.is_integral()) {
         const auto* info = this->_integral_types.lookup_type_info(type.as_integral());
         if (info)
            return &info->options;
      } else if (type.is_container()) {
         const auto* info = this->_container_types.lookup_type_info(type.as_container());
         if (info)
            return &info->options;
      }
      return nullptr;
   }
   
   void report_generator::_member_options_to_xml(
      xml_element& node,
      const bitpacking::data_options& member_options,
      const bitpacking::data_options* type_options,
      size_t member_bitfield_width
   ) {
      if (member_options.union_member_id.has_value()) {
         //
         // For members of a tagged union.
         //
         node.set_attribute_i("union-tag-value", *member_options.union_member_id);
      }
      bitpacking_default_value_to_xml(node, member_options);
      //
      // Handle bitpacking options.
      //
      {
         //
         // Node name.
         //
         if (member_options.is_omitted) {
            node.node_name = "omitted";
         } else {
            node.node_name = "unknown";
            if (member_options.is<typed_options::boolean>()) {
               node.node_name = "boolean";
            } else if (member_options.is<typed_options::buffer>()) {
               node.node_name = "buffer";
            } else if (member_options.is<typed_options::integral>()) {
               node.node_name = "integer";
            } else if (member_options.is<typed_options::pointer>()) {
               node.node_name = "pointer";
            } else if (member_options.is<typed_options::string>()) {
               node.node_name = "string";
            } else if (member_options.is<typed_options::structure>()) {
               node.node_name = "struct";
            } else if (member_options.is<typed_options::tagged_union>()) {
               auto& casted = member_options.as<typed_options::tagged_union>();
               if (casted.is_internal)
                  node.node_name = "union-internal-tag";
               else
                  node.node_name = "union-external-tag";
            } else if (member_options.is<typed_options::transformed>()) {
               node.node_name = "transformed";
            } else {
               node.node_name = "unknown";
            }
         }
         //
         // Options as attributes.
         //
         if (member_options.is<typed_options::buffer>()) {
            const auto& casted = member_options.as<typed_options::buffer>();
            node.set_attribute_i("bytecount", casted.bytecount);
         } else if (member_options.is<typed_options::integral>()) {
            const auto& casted = member_options.as<typed_options::integral>();
            //
            // We generate a c-type node for integral types, listing the 
            // integral options on the type as appropriate. We don't want 
            // to list those options for each individual value if they do 
            // not differ from the type.
            //
            bool serialize_bitcount = true;
            bool serialize_min      = true;
            bool serialize_max      = true;
            if (type_options && type_options->is<typed_options::integral>()) {
               const auto& inherit = type_options->as<typed_options::integral>();
               if (!member_bitfield_width && casted.bitcount == inherit.bitcount)
                  serialize_bitcount = false;
               if (casted.min == inherit.min)
                  serialize_min = false;
               if (casted.max == inherit.max)
                  serialize_max = false;
            }
            if (member_bitfield_width && casted.bitcount == member_bitfield_width)
               serialize_bitcount = false;
            
            if (serialize_bitcount) {
               node.set_attribute_i("bitcount", casted.bitcount);
            }
            if (serialize_min && casted.min != typed_options::integral::no_minimum) {
               node.set_attribute_i("min", casted.min);
            }
            if (serialize_max && casted.max != typed_options::integral::no_maximum) {
               node.set_attribute_i("max", casted.max);
            }
         } else if (member_options.is<typed_options::pointer>()) {
            ;
         } else if (member_options.is<typed_options::string>()) {
            const auto& casted = member_options.as<typed_options::string>();
            node.set_attribute_i("length",    casted.length);
            node.set_attribute_b("nonstring", casted.nonstring);
         } else if (member_options.is<typed_options::structure>()) {
            ;
         } else if (member_options.is<typed_options::tagged_union>()) {
            const auto& casted = member_options.as<typed_options::tagged_union>();
            node.set_attribute("tag", casted.tag_identifier);
         } else if (member_options.is<typed_options::transformed>()) {
            const auto& casted = member_options.as<typed_options::transformed>();
            if (auto opt = casted.transformed_type)
               node.set_attribute("transformed-type", opt->pretty_print());
            if (auto opt = casted.pre_pack)
               node.set_attribute("pack-function",    opt->name());
            if (auto opt = casted.post_unpack)
               node.set_attribute("unpack-function",  opt->name());
         }
      }
      //
      // Bitpacking categories and annotations.
      //
      for(const auto& category : member_options.stat_categories) {
         if (type_options && type_options->has_stat_category(category)) {
            continue;
         }
         auto  elem_ptr = std::make_unique<xml_element>();
         auto& elem     = *elem_ptr;
         elem.node_name = "category";
         elem.set_attribute("name", category);
         node.append_child(std::move(elem_ptr));
      }
      for(const auto& text : member_options.misc_annotations) {
         if (type_options && type_options->has_misc_annotation(text)) {
            continue;
         }
         auto  elem_ptr = std::make_unique<xml_element>();
         auto& elem     = *elem_ptr;
         elem.node_name = "annotation";
         elem.set_attribute("text", text);
         node.append_child(std::move(elem_ptr));
      }
   }
   
   owned_element report_generator::_make_member_element(gw::decl::field field) {
      auto  type = field.value_type();
      auto& desc = codegen::decl_dictionary::get().describe(field);
      
      bool is_bespoke_struct_type = false;
      const bitpacking::data_options* type_options = this->_get_serialized_type_options(type);
      
      auto  node_ptr = std::make_unique<xml_element>();
      auto& node     = *node_ptr;
      node.node_name = "field";
      node.set_attribute("name", field.name());
      {
         auto type = field.value_type();
         auto pp   = type.pretty_print();
         if (pp != "<unnamed>") {
            node.set_attribute("type", pp);
         } else {
            if (type.is_container()) {
               is_bespoke_struct_type = true;
            }
         }
      }
      std::optional<size_t> bitfield_width;
      {  // Field info
         size_t offset = field.offset_in_bits();
         size_t width  = field.size_in_bits();
         if (field.is_bitfield()) {
            bitfield_width = width;
            node.set_attribute_i("c-bitfield-offset", offset);
            node.set_attribute_i("c-bitfield-width",  width);
         }
      }
      for(auto size : desc.array.extents) {
         auto  elem_ptr = std::make_unique<xml_element>();
         auto& elem     = *elem_ptr;
         elem.node_name = "array-rank";
         elem.set_attribute_i("extent", size);
         node.append_child(std::move(elem_ptr));
      }
      this->_member_options_to_xml(
         node,
         desc.options,
         type_options,
         bitfield_width.has_value() ? *bitfield_width : 0
      );
      if (is_bespoke_struct_type) {
         auto members_ptr = this->_referenceable_aggregate_members_to_xml(type.as_container());
         node.append_child(std::move(members_ptr));
      }
      
      return node_ptr;
   }
   owned_element report_generator::_referenceable_aggregate_members_to_xml(gw::type::container type) {
      auto  node_ptr = std::make_unique<xml_element>();
      auto& node     = *node_ptr;
      node.node_name = "members";
      
      type.for_each_referenceable_field([this, &node](gw::decl::field field) {
         node.append_child(this->_make_member_element(field));
      });
      
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
      {
         instruction_tree_xml_generator gen;
         info.instructions = gen.generate(root);
      }
   }
   void report_generator::process(const codegen::whole_struct_function_dictionary& dict) {
      dict.for_each([this](gw::type::base type, const codegen::whole_struct_function_info& ws_info) {
         assert(type.is_container() && "We shouldn't be generating whole-struct functions for non-container types.");
         auto& info = this->_container_types.index_type(type.as_container());
         {
            const auto& root = *ws_info.instructions_root->as<codegen::instructions::container>();
            instruction_tree_xml_generator gen;
            info.instructions = gen.generate(root);
         }
         
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
         if (type.is_container()) {
            auto& info = this->_container_types.index_type(type.as_container());
            info.stats = stats;
            this->_integral_types.index_types_in(type.as_container());
         }
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
            this->_container_types.sort_all();
            this->_container_types.for_each_type_info([this, &out](auto& info) {
               auto  node_ptr = info.stats.to_xml();
               auto& node     = *node_ptr;
               bitpacking_x_options_to_xml(node, info.options, true);
               for(const auto& category : info.options.stat_categories) {
                  auto  elem_ptr = std::make_unique<xml_element>();
                  auto& elem     = *elem_ptr;
                  elem.node_name = "category";
                  elem.set_attribute("name", category);
                  node.append_child(std::move(elem_ptr));
               }
               for(const auto& text : info.options.misc_annotations) {
                  auto  elem_ptr = std::make_unique<xml_element>();
                  auto& elem     = *elem_ptr;
                  elem.node_name = "annotation";
                  elem.set_attribute("text", text);
                  node.append_child(std::move(elem_ptr));
               }
               if (info.stats.type.is_container()) {
                  auto cont      = info.stats.type.as_container();
                  auto child_ptr = this->_referenceable_aggregate_members_to_xml(cont);
                  node.append_child(std::move(child_ptr));
               }
               
               xml_element* instr = nullptr;
               if (info.instructions) {
                  instr = info.instructions.get();
                  node.append_child(std::move(info.instructions));
               }
               out += node.to_string(2);
               out += '\n';
               if (instr) {
                  info.instructions = node.take_child(*instr);
               }
            });
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
            
            const bitpacking::data_options* type_options = nullptr;
            if (ident.serialized_type) {
               type_options = this->_get_serialized_type_options(*ident.serialized_type);
            }
            this->_member_options_to_xml(
               node,
               ident.options,
               type_options,
               0
            );
            
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