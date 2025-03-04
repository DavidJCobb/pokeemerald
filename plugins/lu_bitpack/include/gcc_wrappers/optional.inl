#pragma once
#include "gcc_wrappers/optional.h"

#define CLASS_TEMPLATE_PARAMS template<typename Wrapper>
#define CLASS_NAME optional<Wrapper>

// GCC 12.1.0+ warns on using `std::is_constant_evaluated()` in a 
// non-constexpr function (-Wtautological-compare). How annoying. 
// Given a choice between removing code that plans ahead, or just 
// using a dumb macro to hide it, after a picosecond of thought I 
// have chosen to use a dumb macro.
#define GCC_WRAPPERS_ARE_WE_CONSTEXPR_CAPABLE 0

namespace gcc_wrappers {
   CLASS_TEMPLATE_PARAMS
   void CLASS_NAME::_set_tree(tree raw) {
      #if GCC_WRAPPERS_ARE_WE_CONSTEXPR_CAPABLE
         if (std::is_constant_evaluated()) {
            if (raw == NULL_TREE) {
               this->_tree = NULL_TREE;
            } else {
               this->_node = value_type::wrap(raw);
            }
            return;
         }
      #endif
      if constexpr (impl::has_typecheck<value_type>) {
         if (raw != NULL_TREE) {
            impl::wrap_fail_on_wrong_type<value_type>(raw);
         }
      }
      this->_tree = raw;
   }
   
   // Copy-construct from optional.
   CLASS_TEMPLATE_PARAMS
   template<typename Subclass>
   constexpr CLASS_NAME::optional(const optional<Subclass>& o)
      requires std::is_base_of_v<typename CLASS_NAME::value_type, Subclass>
   {
      this->_set_tree((tree)o.unwrap()); // unwrapped may be const
   }
   
   // Move-construct from optional.
   CLASS_TEMPLATE_PARAMS
   template<typename Subclass>
   constexpr CLASS_NAME::optional(optional<Subclass>&& o) noexcept
      requires std::is_base_of_v<typename CLASS_NAME::value_type, Subclass>
   {
      this->_set_tree((tree)o.unwrap()); // unwrapped may be const
      o._tree = NULL_TREE;
   }
   
   // Construct from wrapper.
   CLASS_TEMPLATE_PARAMS
   template<typename Subclass>
   constexpr CLASS_NAME::optional(const Subclass& o)
      requires std::is_base_of_v<typename CLASS_NAME::value_type, Subclass>
   {
      this->_set_tree((tree)o.unwrap()); // unwrapped may be const
   }
   
   // Construct from bare node pointer.
   CLASS_TEMPLATE_PARAMS
   CLASS_NAME::optional(tree t) {
      this->_set_tree(t);
   }
   
   CLASS_TEMPLATE_PARAMS
   constexpr bool CLASS_NAME::empty() const noexcept {
      #if GCC_WRAPPERS_ARE_WE_CONSTEXPR_CAPABLE
         #if __cpp_lib_is_within_lifetime >= 202306L
            if (std::is_constant_evaluated()) {
               return std::is_within_lifetime(&this->_tree);
            }
         #endif
      #endif
      return this->_tree == NULL_TREE;
   }
   
   CLASS_TEMPLATE_PARAMS
   constexpr tree CLASS_NAME::unwrap() {
      #if GCC_WRAPPERS_ARE_WE_CONSTEXPR_CAPABLE
      if (std::is_constant_evaluated()) {
         if (empty())
            return NULL_TREE;
         return this->_node.unwrap();
      } else {
      #endif
         return this->_tree;
      #if GCC_WRAPPERS_ARE_WE_CONSTEXPR_CAPABLE
      }
      #endif
   }
   
   CLASS_TEMPLATE_PARAMS
   constexpr const_tree CLASS_NAME::unwrap() const {
      #if GCC_WRAPPERS_ARE_WE_CONSTEXPR_CAPABLE
      if (std::is_constant_evaluated()) {
         if (empty())
            return NULL_TREE;
         return this->_node.unwrap();
      } else {
      #endif
         return this->_tree;
      #if GCC_WRAPPERS_ARE_WE_CONSTEXPR_CAPABLE
      }
      #endif
   }
   
   CLASS_TEMPLATE_PARAMS
   template<typename Subclass>
   constexpr optional<Subclass> CLASS_NAME::as()
      requires std::is_base_of_v<typename CLASS_NAME::value_type, Subclass>
   {
      if (empty())
         return {};
      if constexpr (impl::has_typecheck<Subclass>) {
         if (!Subclass::raw_node_is(this->_node)) {
            return {};
         }
      }
      return optional<Subclass>(this->_node);
   }
   
   CLASS_TEMPLATE_PARAMS
   CLASS_NAME::value_type CLASS_NAME::operator*() {
      assert(!empty());
      return this->_node;
   }
   
   CLASS_TEMPLATE_PARAMS
   const CLASS_NAME::value_type CLASS_NAME::operator*() const {
      assert(!empty());
      return this->_node;
   }
   
   CLASS_TEMPLATE_PARAMS
   CLASS_NAME::value_type* CLASS_NAME::operator->() {
      #if GCC_WRAPPERS_ARE_WE_CONSTEXPR_CAPABLE
      if (std::is_constant_evaluated()) {
         if (empty()) {
            throw "invalid compile-time dereferencing of empty optional";
         }
      } else {
      #endif
         assert(!empty());
      #if GCC_WRAPPERS_ARE_WE_CONSTEXPR_CAPABLE
      }
      #endif
      return &this->_node;
   }
   
   CLASS_TEMPLATE_PARAMS
   const CLASS_NAME::value_type* CLASS_NAME::operator->() const {
      #if GCC_WRAPPERS_ARE_WE_CONSTEXPR_CAPABLE
      if (std::is_constant_evaluated()) {
         if (empty()) {
            throw "invalid compile-time dereferencing of empty optional";
         }
      } else {
      #endif
         assert(!empty());
      #if GCC_WRAPPERS_ARE_WE_CONSTEXPR_CAPABLE
      }
      #endif
      return &this->_node;
   }
   
   CLASS_TEMPLATE_PARAMS
   constexpr CLASS_NAME::operator bool() const noexcept {
      return !empty();
   }
}

#undef CLASS_NAME
#undef CLASS_TEMPLATE_PARAMS

#undef GCC_WRAPPERS_ARE_WE_CONSTEXPR_CAPABLE