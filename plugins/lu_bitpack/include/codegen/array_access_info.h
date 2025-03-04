#pragma once

namespace codegen {
   //
   // Represents access to a single array element, or to a slice of an 
   // array. For example, a serialization item may use this to encode 
   // a slice (`foo[0:5]`) to indicate that we'll be generating a `for` 
   // loop which accesses each element within that slice one time.
   //
   // Functions like `serialization_item::segment::is_array` behave 
   // accordingly: `count > 1` does not necessarily mean that the item 
   // represents one array; it may instead represent multiple elements 
   // that are not, themselves, arrays.
   //
   struct array_access_info {
      constexpr bool operator==(const array_access_info&) const noexcept = default;
      
      size_t start = 0;
      size_t count = 1;
   };
}