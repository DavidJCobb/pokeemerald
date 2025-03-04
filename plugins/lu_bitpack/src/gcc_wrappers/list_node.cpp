#include "gcc_wrappers/list_node.h"
#include <stdexcept>
#include "gcc_wrappers/_node_boilerplate-impl.define.h"

namespace gcc_wrappers {
   GCC_NODE_WRAPPER_BOILERPLATE(list_node)
   
   //
   // list_node::iterator
   //
   
   list_node::iterator::iterator(optional_node n) : _node(n) {}
   
   std::pair<optional_node, optional_node> list_node::iterator::operator*() {
      return std::pair(TREE_PURPOSE(this->_node.unwrap()), TREE_VALUE(this->_node.unwrap()));
   }

   list_node::iterator& list_node::iterator::operator++() {
      assert(this->_node);
      this->_node = TREE_CHAIN(this->_node.unwrap());
      return *this;
   }
   list_node::iterator list_node::iterator::operator++(int) const {
      auto it = *this;
      ++it;
      return it;
   }
   
   void list_node::iterator::replace_key(optional_node n) {
      TREE_PURPOSE(this->_node.unwrap()) = n.unwrap();
   }
   void list_node::iterator::replace_value(optional_node n) {
      TREE_VALUE(this->_node.unwrap()) = n.unwrap();
   }
   
   //
   // list_node
   //
   
   /*static*/ bool list_node::raw_node_is(tree t) {
      return TREE_CODE(t) == TREE_LIST;
   }
   
   list_node::list_node(optional_node k, optional_node v) {
      this->_node = tree_cons(k.unwrap(), v.unwrap(), NULL_TREE);
   }
   
   list_node::iterator list_node::begin() {
      return iterator(this->_node);
   }
   list_node::iterator list_node::end() {
      return iterator(nullptr);
   }
   list_node::iterator list_node::at(size_t n) {
      return iterator(this->nth_pair(n));
   }
   
   size_t list_node::size() const {
      return list_length(this->_node);
   }
   
   node list_node::front() {
      return node::wrap(this->_node);
   }
   node list_node::back() {
      return node::wrap(tree_last(this->_node));
   }
   node list_node::nth_pair(size_t n) {
      size_t i = 0;
      for(auto raw = this->_node; raw != NULL_TREE; raw = TREE_CHAIN(raw)) {
         if (i++ == n)
            return node::wrap(raw);
      }
      throw std::out_of_range("out-of-bounds indexed access to `list_node`");
   }
   optional_node list_node::nth_key(size_t n) {
      return TREE_PURPOSE(nth_pair(n).unwrap());
   }
   optional_node list_node::nth_value(size_t n) {
      return TREE_VALUE(nth_pair(n).unwrap());
   }
   
   optional_node list_node::find_value_by_key(optional_node key) {
      auto raw = purpose_member(key.unwrap(), this->_node);
      if (raw != NULL_TREE)
         raw = TREE_VALUE(raw);
      return raw;
   }
   optional_node list_node::find_pair_by_key(optional_node key) {
      return purpose_member(key.unwrap(), this->_node);
   }
   optional_node list_node::find_pair_by_value(optional_node value) {
      return value_member(value.unwrap(), this->_node);
   }
   
   //
   // Functions for mutating a list_node:
   //
   
   void list_node::append(optional_node key, optional_node value) {
      auto node = this->_node;
      auto prev = NULL_TREE;
      while (node != NULL_TREE) {
         prev = node;
         node = TREE_CHAIN(node);
      }
      auto item = tree_cons(key.unwrap(), value.unwrap(), NULL_TREE);
      if (prev == NULL_TREE) {
         this->_node = item;
      } else {
         TREE_CHAIN(prev) = item;
      }
   }
   void list_node::concat(list_node&& other) {
      this->_node = chainon(this->_node, other._node);
   }
   void list_node::prepend(optional_node key, optional_node value) {
      //
      // Insert a new node between the head and its next-sibling; move 
      // all data from the head to the new node; then replace the head 
      // node's data with the to-be-prepended key and value.
      //
      tree head = this->_node;
      tree next = TREE_CHAIN(head);
      
      tree shim = tree_cons(TREE_PURPOSE(head), TREE_VALUE(head), next);
      TREE_CHAIN(head)   = shim;
      TREE_PURPOSE(head) = key.unwrap();
      TREE_VALUE(head)   = value.unwrap();
   }
}