#pragma once
#include "gcc_wrappers/decl/field.h"
#include "gcc_wrappers/decl/variable.h"
#include "gcc_wrappers/type/base.h"
#include "gcc_wrappers/chain.h"
#include "gcc_wrappers/_node_boilerplate.define.h"

namespace gcc_wrappers {
   namespace decl {
      class field;
   }
   class chain_node;
}

namespace gcc_wrappers::type {
   class container : public base {
      public:
         static bool raw_node_is(tree t) {
            switch (TREE_CODE(t)) {
               case RECORD_TYPE:
               case UNION_TYPE:
                  return true;
               default:
                  break;
            }
            return false;
         }
         GCC_NODE_WRAPPER_BOILERPLATE(container)
         
      public:
         chain member_chain() const; // returns TREE_FIELDS(_node)
         
         // Loop over each FIELD_DECL in this object. This will only 
         // catch direct members and not, say, the contents of an 
         // anonymous object member.
         template<typename Functor>
         void for_each_field(Functor&& functor) const;
         
         // Loop over each FIELD_DECL in this object, and recursively 
         // loop over the FIELD_DECLs of any contained anonymous objects.
         template<typename Functor>
         void for_each_referenceable_field(Functor&& functor) const;
         
         template<typename Functor>
         void for_each_static_data_member(Functor&& functor) const;
         
         // GCC parse state.
         bool is_still_being_defined() const;
   };
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(container);
}

#include "gcc_wrappers/_node_boilerplate.undef.h"
#include "gcc_wrappers/type/container.inl"