#pragma once
#include <memory>
#include <string>
#include <vector>
#include "bitpacking/data_options.h"
#include "codegen/stats/category.h"
#include "codegen/stats/sector.h"
#include "codegen/decl_descriptor_pair.h"
#include "xmlgen/container_type_index.h"
#include "xmlgen/integral_type_index.h"
#include "xmlgen/xml_element.h"
#include "gcc_wrappers/type/base.h"
#include "gcc_wrappers/type/container.h"
#include "gcc_wrappers/type/integral.h"
namespace codegen {
   class generation_request;
}

namespace codegen {
   namespace instructions {
      class container;
   }
   class stats_gatherer;
   class value_path;
   class whole_struct_function_dictionary;
}

namespace xmlgen {
   class report_generator {
      public:
         using owned_element = std::unique_ptr<xml_element>;
      
      protected:
         struct category_info {
            std::string              name;
            codegen::stats::category stats;
         };
         struct sector_info {
            codegen::stats::sector stats;
            owned_element          instructions;
         };
         struct top_level_identifier {
            std::string identifier;
            size_t      dereference_count = 0;
            bool        force_to_next_sector = false;
            
            gcc_wrappers::type::optional_base original_type;
            gcc_wrappers::type::optional_base serialized_type;
            bitpacking::data_options options;
         };
         
         std::vector<category_info> _categories;
         std::vector<sector_info>   _sectors;
         container_type_index       _container_types;
         integral_type_index        _integral_types;
         std::vector<top_level_identifier> _top_level_identifiers;
         
      protected:
         category_info& _get_or_create_category_info(std::string_view name);
         
         // Retrieve bitpacking data options for a given type, if that type 
         // (and by extension, its options) will be present in the XML output. 
         // This is used for member elements, so we can avoid having each 
         // member list options that are unchanged from its type.
         const bitpacking::data_options* _get_serialized_type_options(gcc_wrappers::type::base);
         
         void _member_options_to_xml(
            xml_element&,
            const bitpacking::data_options& member_options,
            const bitpacking::data_options* type_options,
            size_t member_bitfield_width = 0 // use 0 if it's not a bitfield
         );
         
         owned_element _make_member_element(gcc_wrappers::decl::field);
         owned_element _referenceable_aggregate_members_to_xml(gcc_wrappers::type::container);
         
         void _sort_category_list();
         
      public:
         void process(const codegen::generation_request&);
         void process(const codegen::instructions::container& sector_root);
         void process(const codegen::whole_struct_function_dictionary&);
         void process(const codegen::stats_gatherer&);
         
         std::string bake();
   };
}