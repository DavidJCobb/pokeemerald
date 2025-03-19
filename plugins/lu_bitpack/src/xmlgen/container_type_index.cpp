#include "xmlgen/container_type_index.h"
#include <algorithm> // std::sort
#include "gcc_wrappers/decl/type_def.h"
namespace gw {
   using namespace gcc_wrappers;
}

namespace xmlgen {
   container_type_index::type_info& container_type_index::index_type(gw::type::container type) {
      for(auto& info : this->_infos)
         if (info.stats.type.is_same(type))
            return info;
      auto& info = this->_infos.emplace_back(type);
      return info;
   }
   
   const container_type_index::type_info* container_type_index::lookup_type_info(gw::type::container type) const noexcept {
      for(auto& info : this->_infos)
         if (info.stats.type.is_same(type))
            return &info;
      return nullptr;
   }
   container_type_index::type_info* container_type_index::lookup_type_info(gw::type::container type) noexcept {
      return const_cast<type_info*>(std::as_const(*this).lookup_type_info(type));
   }
   
   void container_type_index::sort_all() {
      std::sort(
         this->_infos.begin(),
         this->_infos.end(),
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
}