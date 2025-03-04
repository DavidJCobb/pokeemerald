#include <memory> // GCC headers fuck up string-related STL identifiers that <memory> depends on
#include "codegen/rechunked/item.h"
#include "codegen/rechunked/chunks/array_slice.h"
#include "codegen/rechunked/chunks/condition.h"
#include "codegen/rechunked/chunks/padding.h"
#include "codegen/rechunked/chunks/qualified_decl.h"
#include "codegen/rechunked/chunks/transform.h"
#include "codegen/serialization_item.h"
#include "codegen/decl_descriptor.h"

namespace codegen::rechunked {
   item::item(const serialization_item& src) {
      for(size_t i = 0; i < src.segments.size(); ++i) {
         const auto& segm   = src.segments[i];
         const bool  is_end = i == src.segments.size() - 1;
         
         if (segm.condition.has_value()) {
            auto& src_cnd = *segm.condition;
            auto  dst_cnd = std::make_unique<chunks::condition>();
            dst_cnd->rhs     = src_cnd.rhs;
            dst_cnd->is_else = src_cnd.is_else;
            dst_cnd->set_lhs_from_segments(src_cnd.lhs);
            this->chunks.push_back(std::move(dst_cnd));
         }
         
         if (segm.is_padding()) {
            if (!is_end) {
               //
               // If padding needs multiple nested conditions, we hack it by 
               // using multiple padding segments wherein only the last one 
               // defines the actual padding size.
               //
               continue;
            }
            auto chunk_ptr = std::make_unique<chunks::padding>();
            chunk_ptr->bitcount = segm.as_padding().bitcount;
            this->chunks.push_back(std::move(chunk_ptr));
            continue;
         }
         assert(segm.is_basic());
         const auto& data = segm.as_basic();
         
         bool glued = false;
         if (!this->chunks.empty()) {
            auto* back = this->chunks.back()->as<chunks::qualified_decl>();
            if (back) {
               back->descriptors.push_back(data.desc);
               glued = true;
            }
         }
         if (!glued) {
            auto chunk_ptr = std::make_unique<chunks::qualified_decl>();
            chunk_ptr->descriptors.push_back(data.desc);
            this->chunks.push_back(std::move(chunk_ptr));
         }
         
         for(auto& aai : data.array_accesses) {
            auto chunk_ptr = std::make_unique<chunks::array_slice>();
            chunk_ptr->data = aai;
            this->chunks.push_back(std::move(chunk_ptr));
         }
         {
            //
            // Fully expand arrays-of-singles: given `int foo[4][3][2]`, a 
            // serialization item for `foo` should produce the same chunks 
            // as a serialization item for `foo[0:4][0:3][0:2]`. This will 
            // make node trees produced from re-chunked items a bit more 
            // consistent.
            //
            const auto&  ranks = data.desc->array.extents;
            const size_t size  = data.array_accesses.size();
            for(size_t j = size; j < ranks.size(); ++j) {
               auto chunk_ptr = std::make_unique<chunks::array_slice>();
               chunk_ptr->data.start = 0;
               chunk_ptr->data.count = ranks[j];
               this->chunks.push_back(std::move(chunk_ptr));
            }
         }
         
         const auto& types = data.desc->types.transformations;
         if (!types.empty()) {
            auto chunk_ptr = std::make_unique<chunks::transform>();
            chunk_ptr->types = types;
            this->chunks.push_back(std::move(chunk_ptr));
         }
      }
   }
   
   bool item::is_omitted_and_defaulted() const {
      bool omitted   = false;
      bool defaulted = false;
      for(auto& chunk_ptr : this->chunks) {
         if (auto* casted = chunk_ptr->as<chunks::qualified_decl>()) {
            for(auto* desc : casted->descriptors) {
               assert(desc != nullptr);
               if (desc->options.is_omitted)
                  omitted = true;
               if (!!desc->options.default_value)
                  defaulted = true;
               else if (desc->is_or_contains_defaulted())
                  defaulted = true;
               
               if (omitted && defaulted)
                  return true;
            }
         }
      }
      return omitted && defaulted;
   }
}