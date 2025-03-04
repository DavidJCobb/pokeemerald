#include "codegen/serialization_item_list_ops/fold_sequential_array_elements.h"
#include "codegen/decl_descriptor.h"

namespace codegen::serialization_item_list_ops {
   //
   // If there's a set of multiple serialization items in a row that refer 
   // to sequential array elements, fold them into a single serialization 
   // item that refers to an array slice.
   //
   extern void fold_sequential_array_elements(std::vector<serialization_item>& list) {
      size_t size = list.size();
      for(size_t i = 1; i < size; ++i) {
         auto& item = list[i];
         auto& prev = list[i - 1];
         
         if (item.is_padding())
            continue;
         if (prev.is_padding())
            continue;
         if (item.is_omitted != prev.is_omitted)
            continue;
         if (item.is_defaulted != prev.is_defaulted)
            continue;
         
         //
         // Check if all but the last path segment are identical.
         //
         size_t length = item.segments.size();
         if (length != prev.segments.size())
            continue;
         bool equal_stems = true;
         for(size_t j = 0; j < length - 1; ++j) {
            if (item.segments[j] != prev.segments[j]) {
               equal_stems = false;
               break;
            }
         }
         if (!equal_stems)
            continue;
         
         auto& seg_data_i = item.segments.back().as_basic();
         auto& seg_data_p = prev.segments.back().as_basic();
         
         //
         // Check if the last path segment refers to the same DECL.
         //
         if (seg_data_i.desc != seg_data_p.desc)
            continue;
         
         //
         // Check if all but the innermost array access are identical.
         //
         auto&  item_aa = seg_data_i.array_accesses;
         auto&  prev_aa = seg_data_p.array_accesses;
         size_t rank    = item_aa.size();
         if (rank != prev_aa.size())
            continue;
         bool equal_outer_ranks = true;
         for(size_t j = 0; j < rank - 1; ++j) {
            if (item_aa[j] != prev_aa[j]) {
               equal_outer_ranks = false;
               break;
            }
         }
         if (!equal_outer_ranks)
            continue;
         
         //
         // Check if the innermost array accesses are contiguous. Merge the two 
         // serialization items if so, keeping `prev` and destroying `item`.
         //
         if (item_aa.back().start == prev_aa.back().start + prev_aa.back().count) {
            prev_aa.back().count += item_aa.back().count;
            list.erase(list.begin() + i);
            --i;
            --size;
            //
            // If, as a result of the merge, this array slice now represents the 
            // entire array, then ditch the innermost array access. For example, 
            // given `int foo[3][2]`, an access like `foo[0][0:2]` will simplify 
            // to `foo[0]`.
            //
            auto& last_aa = prev_aa.back();
            if (last_aa.start == 0) {
               auto extent = seg_data_p.desc->array.extents[seg_data_p.array_accesses.size() - 1];
               if (last_aa.count == extent) {
                  prev_aa.resize(prev_aa.size() - 1);
               }
            }
            
            continue;
         }
         // Else, not mergeable.
      }
   }
}