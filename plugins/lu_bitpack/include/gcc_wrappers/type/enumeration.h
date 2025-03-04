#pragma once
#include <string_view>
#include <vector>
#include "gcc_wrappers/type/integral.h"
#include "gcc_wrappers/_node_boilerplate.define.h"

namespace gcc_wrappers::type {
   class enumeration : public integral {
      public:
         static bool raw_node_is(tree t) {
            return TREE_CODE(t) == ENUMERAL_TYPE;
         }
         GCC_NODE_WRAPPER_BOILERPLATE(enumeration)
         
      public:
         struct member {
            std::string_view name;
            intmax_t         value = 0;
         };
         
      public:
         [[nodiscard]] std::vector<member> all_enum_members() const;
         
         bool is_opaque() const;
         bool is_scoped() const;
         
         base underlying_type() const;
         bool has_explicit_underlying_type() const;
      
         template<typename Functor>
         void for_each_enum_member(Functor&& functor) const;
         
         // GCC parse state.
         bool is_still_being_defined() const;
   };
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(enumeration);
}

#include "gcc_wrappers/_node_boilerplate.undef.h"
#include "gcc_wrappers/type/enumeration.inl"