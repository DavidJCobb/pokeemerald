#pragma once
#include "lu/strings/zview.h"
#include "gcc_wrappers/node.h"
#include "gcc_wrappers/optional.h"
#include "gcc_wrappers/_node_boilerplate.define.h"

namespace gcc_wrappers {
   class attribute;
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(attribute);
}

namespace gcc_wrappers {
   class attribute_list {
      protected:
         optional_node _head;
      
      public:
         static attribute_list wrap(tree t) {
            attribute_list out;
            out._head = t;
            return out;
         }
         tree unwrap() const {
            return (tree)this->_head.unwrap();
         }
         
         class iterator {
            friend attribute_list;
            protected:
               iterator(optional_node);
               
               optional_node _node = NULL_TREE;
               
            public:
               attribute operator*();
            
               iterator& operator++();
               iterator  operator++(int) const;
               
               bool operator==(const iterator&) const = default;
         };
         
      public:
         // Equality comparison.
         bool operator==(const attribute_list) const noexcept;
         
         bool contains(const attribute) const;
         bool empty() const;
         
         iterator begin();
         iterator end();
         iterator at(size_t n); // may throw std::out_of_range
      
         // Returns the first attribute whose name starts with the given substring.
         optional_attribute first_attribute_with_prefix(lu::strings::zview);
         const optional_attribute first_attribute_with_prefix(lu::strings::zview) const;
      
         // Returns the first attribute with the given name.
         optional_attribute get_attribute(lu::strings::zview);
         const optional_attribute get_attribute(lu::strings::zview) const;
      
         bool has_attribute(lu::strings::zview) const;
         
         // assert(TREE_CODE(id_node) == IDENTIFIER_NODE)
         bool has_attribute(tree id_node) const;
         
         void remove_attribute(lu::strings::zview);
   };
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(attribute_list);
}

#include "gcc_wrappers/_node_boilerplate.undef.h"