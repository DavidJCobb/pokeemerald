#include "xmlgen/integral_type_index.h"
#include "gcc_wrappers/type/array.h"
#include "gcc_wrappers/type/container.h"
namespace gw {
   using namespace gcc_wrappers;
}
namespace typed_options {
   using namespace bitpacking::typed_data_options::computed;
}

namespace xmlgen {
   
   //
   // integral_type_index::type_info
   //
   
   std::unique_ptr<xmlgen::xml_element> integral_type_index::type_info::to_xml() const {
      auto  node_ptr = std::make_unique<xml_element>();
      auto& node     = *node_ptr;
      node.set_attribute("name", this->name);
      if (this->options.is<typed_options::integral>()) {
         const auto& casted = this->options.as<typed_options::integral>();
         node.set_attribute_i("bitcount", casted.bitcount);
         if (casted.min != typed_options::integral::no_minimum) {
            node.set_attribute_i("min", casted.min);
         }
         if (casted.max != typed_options::integral::no_maximum) {
            node.set_attribute_i("max", casted.max);
         }
      }
      for(const auto& category : this->options.stat_categories) {
         auto  elem_ptr = std::make_unique<xml_element>();
         auto& elem     = *elem_ptr;
         elem.node_name = "category";
         elem.set_attribute("name", category);
         node.append_child(std::move(elem_ptr));
      }
      for(const auto& text : this->options.misc_annotations) {
         auto  elem_ptr = std::make_unique<xml_element>();
         auto& elem     = *elem_ptr;
         elem.node_name = "annotation";
         elem.set_attribute("text", text);
         node.append_child(std::move(elem_ptr));
      }
      return node_ptr;
   }
   
   //
   // integral_type_index::canonical_type_info
   //
   
   std::unique_ptr<xmlgen::xml_element> integral_type_index::canonical_type_info::to_xml() const {
      auto  node_ptr = type_info::to_xml();
      auto& node     = *node_ptr;
      node.node_name = "integral";
      if (this->alignment) {
         node.set_attribute_i("alignment", this->alignment);
      }
      node.set_attribute_b("is-signed", this->is_signed);
      if (this->size) {
         node.set_attribute_i("size", this->size);
      }
      
      for(auto& alt : this->typedefs) {
         auto  child_ptr = alt.to_xml();
         auto& child     = *child_ptr;
         node.append_child(std::move(child_ptr));
         child.node_name = "typedef";
      }
      
      return node_ptr;
   }
   
   //
   // integral_type_index
   //
   
   void integral_type_index::_index_typedef(integral_type_index::canonical_type_info& canonical_info, gw::type::integral type) {
      for(const auto& prior : canonical_info.typedefs)
         if (prior.node->is_same(type))
            return;
      auto& item = canonical_info.typedefs.emplace_back();
      item.node = type;
      item.name = type.name();
      item.options.load(type);
   }
   
   void integral_type_index::index_type(gw::type::integral type) {
      auto canonical = type.canonical().as_integral();
      for(auto& info : this->_canonical_infos) {
         if (info.node->is_same(type))
            return;
         if (info.node->is_same(canonical)) {
            this->_index_typedef(info, type);
            return;
         }
      }
      //
      // New canonical type.
      //
      auto& info = this->_canonical_infos.emplace_back();
      info.node = canonical;
      info.name = canonical.name();
      info.is_signed = canonical.is_signed();
      {
         auto align = canonical.alignment_in_bits();
         if (!(align % 8))
            info.alignment = align / 8;
      }
      {
         auto size = canonical.size_in_bits();
         if (!(size % 8))
            info.size = size / 8;
      }
      if (!canonical.is_same(type))
         this->_index_typedef(info, type);
   }
   void integral_type_index::index_types_in(gw::type::container cont) {
      cont.for_each_referenceable_field([this](gw::decl::field field) {
         auto type = field.value_type();
         while (type.is_array())
            type = type.as_array().value_type();
         
         if (type.is_container() && type.name().empty()) {
            this->index_types_in(type.as_container());
            return;
         }
         if (!type.is_integral())
            return;
         
         this->index_type(type.as_integral());
      });
   }
   
   const integral_type_index::canonical_type_info* integral_type_index::lookup_canonical_info(gw::type::integral type) const noexcept {
      for(const auto& info : this->_canonical_infos)
         if (info.node->is_same(type))
            return &info;
      return nullptr;
   }
   const integral_type_index::type_info* integral_type_index::lookup_type_info(gw::type::integral type) const noexcept {
      for(const auto& info : this->_canonical_infos) {
         if (info.node->is_same(type))
            return &info;
         for(const auto& sub : info.typedefs)
            if (sub.node->is_same(type))
               return &sub;
      }
      return nullptr;
   }
   
   // Returns a < b, for the purposes of sorting.
   static bool _compare(
      const integral_type_index::type_info& a,
      const integral_type_index::type_info& b
   ) {
      auto& name_a = a.name;
      auto& name_b = b.name;
      
      size_t end = name_a.size();
      if (end > name_b.size())
         end = name_b.size();
      
      //
      // Sort criteria, in order of descending priority:
      //
      //  - ASCII-case-insensitive comparison
      //  - Length comparison (shorter first)
      //  - ASCII-case-sensitive comparison
      //  - Sort by GCC type-node pointer (last-resort fallback)
      //
      for(size_t i = 0; i < end; ++i) {
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
         return (c & 0x20) == 0; // uppercase below lowercase
      }
      return a.node.unwrap() < b.node.unwrap(); // fallback
   }
   
   void integral_type_index::sort_all() {
      std::sort(
         this->_canonical_infos.begin(),
         this->_canonical_infos.end(),
         &_compare
      );
      for(auto& item : this->_canonical_infos) {
         std::sort(
            item.typedefs.begin(),
            item.typedefs.end(),
            &_compare
         );
      }
   }
}