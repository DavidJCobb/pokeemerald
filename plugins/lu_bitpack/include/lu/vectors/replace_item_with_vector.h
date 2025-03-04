#pragma once
#include <stdexcept>
#include <vector>

namespace lu::vectors {
   template<typename Vector>
   #if __cpp_lib_constexpr_vector >= 201907L
   constexpr
   #endif
   void replace_item_with_vector(Vector& dst, const size_t dst_index, const Vector& src) {
      const size_t size_prior = dst.size();
      if (dst_index >= size_prior)
         throw std::out_of_range("bad element index to replace with another vector");
      
      if (dst_index == size_prior - 1) {
         //
         // Special-case: replace at end.
         //
         dst.resize(size_prior - 1 + src.size());
         std::copy(
            src.begin(),
            src.end(),
            dst.begin() + size_prior - 1
         );
         return;
      }
      
      Vector out;
      out.resize(size_prior - 1 + src.size());
      if (dst_index > 0) {
         std::copy(
            dst.begin(),
            dst.begin() + dst_index,
            out.begin()
         );
      }
      std::copy(
         src.begin(),
         src.end(),
         out.begin() + dst_index
      );
      std::copy(
         dst.begin() + dst_index + 1,
         dst.end(),
         out.begin() + dst_index + src.size()
      );
      
      dst = out;
   }
}