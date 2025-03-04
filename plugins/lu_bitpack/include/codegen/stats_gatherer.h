#pragma once
#include <memory> // GCC headers fuck up string-related STL identifiers that <memory> depends on
#include <unordered_map>
#include <vector>
#include "gcc_wrappers/decl/variable.h"
#include "gcc_wrappers/type/base.h"
#include "codegen/array_access_info.h"
#include "codegen/serialization_item.h"
#include "codegen/stats/category.h"
#include "codegen/stats/c_type.h"
#include "codegen/stats/sector.h"

namespace codegen {
   class decl_descriptor;
}

namespace codegen {
   class stats_gatherer {
      protected:
         struct count_context {
            size_t containing_array_count  = 1;
            size_t containing_sector_index = 0;
            gcc_wrappers::decl::optional_variable containing_variable;
            
            void enter_array(const decl_descriptor&);
            void enter_array_slice(const std::vector<array_access_info>& slices, const decl_descriptor&);
            
            void on_single_serializable_seen(stats::serializable&) const;
         };
         
      public:
         std::unordered_map<std::string, stats::category>            categories;
         std::vector<stats::sector>                                  sectors;
         std::unordered_map<gcc_wrappers::type::base, stats::c_type> types; // keys are un-transformed types
         
      protected:
         void _seen_stats_category_annotation(const count_context&, std::string_view name, const decl_descriptor&);
      
         // Used on the final segment of a serialization item, to gather any 
         // sub-items that could be produced were the item to be expanded.
         void _gather_leaf(const count_context&, const decl_descriptor&);
         
         void _seen_decl(const count_context&, const decl_descriptor&);
         
      public:
         void gather_from_sectors(const std::vector<std::vector<serialization_item>>& items_by_sector);
   };
}