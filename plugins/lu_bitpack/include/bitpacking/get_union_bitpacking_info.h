#pragma once
#include <optional>
#include <utility>
#include "lu/strings/zview.h"
#include "gcc_wrappers/decl/field.h"
#include "gcc_wrappers/type/base.h"

namespace bitpacking {
   struct union_bitpacking_info {
      struct tag_info {
         lu::strings::zview          identifier; // empty if the attribute was invalid
         gcc_wrappers::optional_node specifying_node; // for diagnostic purposes
      };
      
      std::optional<tag_info> external;
      std::optional<tag_info> internal;
      
      constexpr bool empty() const noexcept {
         return !external.has_value() && !internal.has_value();
      }
   };
   
   // Used for error-checking unions whenever we're able to do so.
   // `type` is not specifically an `untagged_union` because we want 
   // to support specifying options on arrays of unions.
   extern union_bitpacking_info get_union_bitpacking_info(
      gcc_wrappers::decl::optional_field decl,
      gcc_wrappers::type::optional_base  type
   );
}