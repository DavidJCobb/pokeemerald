#pragma once
#include <memory>
#include <string>
#include <vector>
#include "gcc_wrappers/type/integral.h"
#include "bitpacking/data_options.h"
#include "xmlgen/xml_element.h"

namespace xmlgen {
   class integral_type_index {
      public:
         struct type_info {
            gcc_wrappers::type::optional_integral node;
            std::string name;
            bitpacking::data_options options;
            
            std::unique_ptr<xmlgen::xml_element> to_xml() const;
         };
         
         struct canonical_type_info : public type_info {
            std::vector<type_info> typedefs;
            
            size_t alignment = 0; // in bytes; 0 if not measurable in bytes
            bool   is_signed = false;
            size_t size      = 0; // in bytes; 0 if not measurable in bytes
            
            std::unique_ptr<xmlgen::xml_element> to_xml() const;
         };
         
      protected:
         std::vector<canonical_type_info> _canonical_infos;
         
         void _index_typedef(canonical_type_info&, gcc_wrappers::type::integral);
         
      public:
         void index_type(gcc_wrappers::type::integral);
         void index_types_in(gcc_wrappers::type::container);
         
         const canonical_type_info* lookup_canonical_info(gcc_wrappers::type::integral) const noexcept;
         const type_info* lookup_type_info(gcc_wrappers::type::integral) const noexcept;
         
         void sort_all();
         
         template<typename Functor>
         void for_each_canonical_info(Functor&& functor) const {
            for(const auto& info : this->_canonical_infos)
               (functor)(info);
         }
   };
}