#pragma once
#include <memory>
#include <vector>
#include "gcc_wrappers/type/container.h"
#include "bitpacking/data_options.h"
#include "codegen/stats/c_type.h"
#include "xmlgen/xml_element.h"

namespace xmlgen {
   class container_type_index {
      public:
         using owned_element = std::unique_ptr<xml_element>;
         
         struct type_info {
            type_info(gcc_wrappers::type::container t) : stats(t) {
               this->options.load(t);
            }
            
            bitpacking::data_options options;
            codegen::stats::c_type   stats;
            owned_element            instructions;
         };
         
      protected:
         std::vector<type_info> _infos;
         
      public:
         type_info& index_type(gcc_wrappers::type::container);
         
         const type_info* lookup_type_info(gcc_wrappers::type::container) const noexcept;
         type_info* lookup_type_info(gcc_wrappers::type::container) noexcept;
         
         void sort_all();
         
         template<typename Functor>
         void for_each_type_info(Functor&& functor) const {
            for(const auto& info : this->_infos)
               (functor)(info);
         }
         template<typename Functor>
         void for_each_type_info(Functor&& functor) {
            for(auto& info : this->_infos)
               (functor)(info);
         }
   };
}