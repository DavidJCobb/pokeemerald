#pragma once

namespace codegen {
   class decl_descriptor;
}

namespace codegen {
   struct decl_descriptor_pair {
      const decl_descriptor* read = nullptr;
      const decl_descriptor* save = nullptr;
      
      constexpr bool operator==(const decl_descriptor_pair&) const noexcept = default;
   };
}