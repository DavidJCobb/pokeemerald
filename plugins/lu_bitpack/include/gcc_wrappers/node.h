#pragma once
#include <cassert>
#include <concepts>
#include <string_view>
#include <type_traits>
#include <version>
// GCC:
#include <gcc-plugin.h>
#include <tree.h>

namespace gcc_wrappers {
   namespace impl {
      template<typename Wrapper>
      concept has_typecheck = requires (tree t) {
         { Wrapper::raw_node_is(t) } -> std::same_as<bool>;
      };
      
      template<typename Super, typename Sub>
      concept can_is_as = std::is_base_of_v<Super, Sub> && !std::is_same_v<Super, Sub> && has_typecheck<Sub>;
      
      void wrap_fail_on_null(tree);
      
      [[noreturn]] void _wrap_report_wrong_type(tree);
      //
      template<typename Wrapper>
      void wrap_fail_on_wrong_type(tree t) {
         if constexpr (has_typecheck<Wrapper>) {
            if (!Wrapper::raw_node_is(t)) {
               _wrap_report_wrong_type(t);
            }
         }
      }
   }
   
   class node {
      protected:
         tree _node = NULL_TREE;
         
         node() {}
         
      public:
         static node wrap(tree n);
         
         // Identity comparison.
         constexpr bool is_same(const node&) const noexcept;
         
         // Identity comparison by default. Some subclasses may override this 
         // to do an equality comparison instead, which is why `is_same` has 
         // been provided separately.
         constexpr bool operator==(const node&) const noexcept;
      
         tree_code code() const;
         const std::string_view code_name() const noexcept;
         
         constexpr const_tree unwrap() const;
         constexpr tree unwrap();
         
         template<typename Subclass> requires impl::can_is_as<node, Subclass>
         bool is() const noexcept {
            return Subclass::raw_node_is(this->_node);
         }
         
         template<typename Subclass> requires impl::can_is_as<node, Subclass>
         Subclass as() {
            assert(is<Subclass>());
            return Subclass::wrap(this->_node);
         }
   };
}

#include "gcc_wrappers/node.inl"

// std::hash isn't declared in any one header, so just pick and include a 
// random header that's known to declare it:
#include <optional>

// allow wrapped nodes as keys in containers like `std::unordered_map`
namespace std {
   template<typename T> requires std::is_base_of_v<gcc_wrappers::node, T>
   struct hash<T> {
      size_t operator()(const T& s) const noexcept {
         return hash<void*>()((void*)s.unwrap());
      }
   };
}