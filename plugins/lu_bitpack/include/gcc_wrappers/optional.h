#pragma once
#include <cassert>
#include <type_traits>
#include "gcc_wrappers/node.h"
// GCC:
#include <gcc-plugin.h>
#include <tree.h>

namespace gcc_wrappers {
   template<typename Wrapper>
   class optional {
      public:
         using value_type = Wrapper;
      
      protected:
         //
         // Doing it this way avoids an ugly cast in `operator->`, and 
         // allows us to plan ahead in the (admittedly unlikely) event 
         // that GCC becomes constexpr-friendly enough for us to (with 
         // C++26 features) be viable in constexpr.
         //
         // (We'd still have to make a bunch of the `node` machinery 
         // constexpr as well, though...)
         //
         union {
            value_type _node;
            tree       _tree = NULL_TREE;
         };
         
         void _set_tree(tree);
         
      public:
         constexpr optional() {}
         
         // Copy-construct from optional.
         template<typename Subclass>
         constexpr optional(const optional<Subclass>& o)
            requires std::is_base_of_v<value_type, Subclass>;
         
         // Move-construct from optional.
         template<typename Subclass>
         constexpr optional(optional<Subclass>&& o) noexcept
            requires std::is_base_of_v<value_type, Subclass>;
         
         // Construct from wrapper.
         template<typename Subclass>
         constexpr optional(const Subclass& o)
            requires std::is_base_of_v<value_type, Subclass>;
         
         // Construct from bare node pointer. If the bare node isn't 
         // NULL_TREE and `value_type::raw_node_is` is defined, then 
         // we assert that the bare node is of the right type.
         optional(tree t);
         
         template<typename Other>
         constexpr bool operator==(const optional<Other>& o) const noexcept {
            return this->unwrap() == o.unwrap();
         }
         
         constexpr bool operator==(const_tree o) const noexcept {
            return this->_node == o;
         }
         
         constexpr bool empty() const noexcept;
         
         constexpr tree unwrap();
         constexpr const_tree unwrap() const;
         
         // Cast to subclass type. If we hold a node which is not of 
         // that type, we return an empty optional.
         template<typename Subclass>
         constexpr optional<Subclass> as() requires std::is_base_of_v<value_type, Subclass>;
      
         value_type operator*();
         const value_type operator*() const;
         
         value_type* operator->();
         const value_type* operator->() const;
         
         constexpr operator bool() const noexcept;
   };
   
   using optional_node = optional<node>;
}

#include "gcc_wrappers/optional.inl"