#pragma once
#include <utility> // std:pair
#include "gcc_wrappers/node.h"
#include "gcc_wrappers/optional.h"
#include "gcc_wrappers/_node_boilerplate.define.h"

namespace gcc_wrappers {
   class list_node : public node {
      public:
         static bool raw_node_is(tree t);
         GCC_NODE_WRAPPER_BOILERPLATE(list_node)
         
         class iterator {
            friend list_node;
            protected:
               iterator(optional_node);
               
               optional_node _node;
               
            public:
               std::pair<optional_node, optional_node> operator*(); // k, v
            
               iterator& operator++();
               iterator  operator++(int) const;
               
               bool operator==(const iterator&) const = default;
               
               void replace_key(optional_node);
               void replace_value(optional_node);
         };
         
      public:
         // An ugly wart of this API is that creating a list also creates a list 
         // head, i.e. you can't have an "empty list."
         list_node(optional_node key, optional_node value);
         
         iterator begin();
         iterator end();
         iterator at(size_t n);
         
         size_t size() const;
         
         node          front(); // first k/v pair
         node          back(); // last k/v pair
         node          nth_pair(size_t n);
         optional_node nth_key(size_t n);
         optional_node nth_value(size_t n);
         
         optional_node find_value_by_key(optional_node key);
         optional_node find_pair_by_key(optional_node key);
         optional_node find_pair_by_value(optional_node value);
         
         // run the functor for each item's TREE_PURPOSE
         template<typename Functor>
         void for_each_key(Functor&&);
         
         // run the functor for each item's TREE_VALUE
         template<typename Functor>
         void for_each_value(Functor&&);
         
         // run the functor for each item's TREE_PURPOSE and TREE_VALUE
         template<typename Functor>
         void for_each_kv_pair(Functor&&);
         
         //
         // Functions for mutating a list:
         //
         
         void append(optional_node key, optional_node value);
         void concat(list_node&&);
         void prepend(optional_node key, optional_node value);
   };
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(list_node);
}

#include "gcc_wrappers/_node_boilerplate.undef.h"
#include "gcc_wrappers/list_node.inl"