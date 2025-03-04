#include "codegen/rechunked/chunks/array_slice.h"
#include "codegen/rechunked/chunks/condition.h"
#include "codegen/rechunked/chunks/qualified_decl.h"
#include "codegen/rechunked/chunks/transform.h"
#include "codegen/serialization_items/basic_segment.h"
#include "codegen/decl_descriptor.h"

namespace codegen::rechunked::chunks {
   /*virtual*/ bool condition::compare(const base& other) const noexcept /*override*/ {
      const auto* casted = other.as<condition>();
      if (!casted)
         return false;
      
      if (this->rhs != casted->rhs)
         return false;
      if (this->is_else != casted->is_else)
         return false;
      if (!this->compare_lhs(*casted))
         return false;
      
      return true;
   }
   
   void condition::set_lhs_from_segments(const std::vector<serialization_items::basic_segment>& src) {
      this->lhs.chunks.clear();
      
      for(const auto& segm : src) {
         bool glued = false;
         if (!this->lhs.chunks.empty()) {
            auto* back = this->lhs.chunks.back()->as<chunks::qualified_decl>();
            if (back) {
               back->descriptors.push_back(segm.desc);
               glued = true;
            }
         }
         if (!glued) {
            auto chunk_ptr = std::make_unique<chunks::qualified_decl>();
            chunk_ptr->descriptors.push_back(segm.desc);
            this->lhs.chunks.push_back(std::move(chunk_ptr));
         }
          
         for(auto& aai : segm.array_accesses) {
            auto chunk_ptr = std::make_unique<chunks::array_slice>();
            chunk_ptr->data = aai;
            this->lhs.chunks.push_back(std::move(chunk_ptr));
         }
         
         if (!segm.desc->types.transformations.empty()) {
            auto chunk_ptr = std::make_unique<chunks::transform>();
            chunk_ptr->types = segm.desc->types.transformations;
            this->lhs.chunks.push_back(std::move(chunk_ptr));
         }
      }
   }
   
   bool condition::compare_lhs(const condition& other) const noexcept {
      size_t size = this->lhs.chunks.size();
      if (size != other.lhs.chunks.size())
         return false;
      for(size_t i = 0; i < size; ++i) {
         const auto* a = this->lhs.chunks[i].get();
         const auto* b = other.lhs.chunks[i].get();
         assert(a != nullptr);
         assert(b != nullptr);
         if (!a->compare(*b))
            return false;
      }
      return true;
   }
}