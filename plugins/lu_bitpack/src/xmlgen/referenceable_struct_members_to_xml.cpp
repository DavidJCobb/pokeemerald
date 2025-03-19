#include "xmlgen/referenceable_struct_members_to_xml.h"
#include "xmlgen/bitpacking_default_value_to_xml.h"
#include "codegen/decl_descriptor.h"
#include "codegen/decl_dictionary.h"
#include "gcc_wrappers/decl/field.h"
#include "gcc_wrappers/decl/type_def.h"
namespace gw {
   using namespace gcc_wrappers;
}

#include "bitpacking/data_options.h"
#include "gcc_wrappers/type/array.h"
#include "xmlgen/bitpacking_default_value_to_xml.h"
#include "xmlgen/bitpacking_x_options_to_xml.h"
#include "xmlgen/integral_type_index.h"
namespace typed_options {
   using namespace bitpacking::typed_data_options::computed;
}

namespace xmlgen {
   extern std::unique_ptr<xml_element> referenceable_struct_members_to_xml(
      gw::type::container type,
      const integral_type_index* known_integral_types
   ) {
      auto& dict = codegen::decl_dictionary::get();
      
      auto  node_ptr = std::make_unique<xml_element>();
      auto& node     = *node_ptr;
      node.node_name = "members";
      
      type.for_each_referenceable_field([&dict, &node, known_integral_types](gw::decl::field field) {
         bool is_bespoke_struct_type = false; // i.e. `struct {} foo;`
         
         const integral_type_index::type_info* integral_type_info = nullptr;
         
         auto  child_ptr = std::make_unique<xml_element>();
         auto& child     = *child_ptr;
         child.node_name = "field";
         child.set_attribute("name", field.name());
         {
            auto type = field.value_type();
            auto pp   = type.pretty_print();
            if (pp != "<unnamed>") {
               child.set_attribute("type", pp);
            } else {
               if (type.is_container()) {
                  is_bespoke_struct_type = true;
               }
            }
            //
            // Fetch integral type info.
            //
            while (type.is_array())
               type = type.as_array().value_type();
            if (type.is_integral() && known_integral_types) {
               integral_type_info = known_integral_types->lookup_type_info(type.as_integral());
               if (integral_type_info)
                  if (!integral_type_info->options.is<typed_options::integral>())
                     integral_type_info = nullptr;
            }
         }
         std::optional<size_t> bitfield_width;
         {  // Field info
            size_t offset = field.offset_in_bits();
            size_t width  = field.size_in_bits();
            if (field.is_bitfield()) {
               bitfield_width = width;
               child.set_attribute_i("c-bitfield-offset", offset);
               child.set_attribute_i("c-bitfield-width",  width);
            }
         }
         {  // Bitpacking info and options
            const auto& desc = dict.describe(field);
            for(auto size : desc.array.extents) {
               auto  elem_ptr = std::make_unique<xml_element>();
               auto& elem     = *elem_ptr;
               elem.node_name = "array-rank";
               elem.set_attribute_i("extent", size);
               child.append_child(std::move(elem_ptr));
            }
            const auto& options = desc.options;
            if (options.union_member_id.has_value()) {
               child.set_attribute_i("union-tag-value", *options.union_member_id);
            }
            bitpacking_default_value_to_xml(child, options);
            bitpacking_x_options_to_xml(child, options, false);
            if (options.is<typed_options::integral>()) {
               const auto& casted_a = options.as<typed_options::integral>();
               //
               // HACK: Clear redundant attributes.
               //
               size_t type_bitcount  = 0;
               size_t field_bitcount = 0;
               if (integral_type_info) {
                  const auto& casted_b = integral_type_info->options.as<typed_options::integral>();
                  if (casted_a.bitcount == casted_b.bitcount) {
                     type_bitcount = casted_b.bitcount;
                  }
                  if (casted_a.min == casted_b.min) {
                     child.remove_attribute("min");
                  }
                  if (casted_a.max == casted_b.max) {
                     child.remove_attribute("max");
                  }
               }
               if (bitfield_width.has_value()) {
                  field_bitcount = *bitfield_width;
               } else {
                  field_bitcount = type_bitcount;
               }
               if (field_bitcount) {
                  if (casted_a.bitcount == field_bitcount)
                     child.remove_attribute("bitcount");
               } else if (type_bitcount) {
                  if (casted_a.bitcount == type_bitcount)
                     child.remove_attribute("bitcount");
               }
            }
            //
            // Categories:
            //
            for(const auto& category : options.stat_categories) {
               if (integral_type_info) {
                  bool found = false;
                  for(const auto& exclude : integral_type_info->options.stat_categories) {
                     if (exclude == category) {
                        found = true;
                        break;
                     }
                  }
                  if (found)
                     continue;
               }
               auto  elem_ptr = std::make_unique<xml_element>();
               auto& elem     = *elem_ptr;
               elem.node_name = "category";
               elem.set_attribute("name", category);
               child.append_child(std::move(elem_ptr));
            }
            for(const auto& text : options.misc_annotations) {
               if (integral_type_info) {
                  bool found = false;
                  for(const auto& exclude : integral_type_info->options.misc_annotations) {
                     if (exclude == text) {
                        found = true;
                        break;
                     }
                  }
                  if (found)
                     continue;
               }
               auto  elem_ptr = std::make_unique<xml_element>();
               auto& elem     = *elem_ptr;
               elem.node_name = "annotation";
               elem.set_attribute("text", text);
               child.append_child(std::move(elem_ptr));
            }
         }
         if (is_bespoke_struct_type) {
            auto members_ptr = referenceable_struct_members_to_xml(field.value_type().as_container(), known_integral_types);
            child.append_child(std::move(members_ptr));
         }
         node.append_child(std::move(child_ptr));
      });
      
      return node_ptr;
   }
}