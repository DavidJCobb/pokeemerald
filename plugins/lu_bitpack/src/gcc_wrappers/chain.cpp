#include "gcc_wrappers/chain.h"
#include <stdexcept>

namespace gcc_wrappers {
   
   //
   // chain::iterator
   //
   
   chain::iterator::iterator(optional_node n) : _node(n) {}
   
   node chain::iterator::operator*() {
      return *this->_node;
   }

   chain::iterator& chain::iterator::operator++() {
      assert(this->_node);
      this->_node = TREE_CHAIN(this->_node.unwrap());
      return *this;
   }
   chain::iterator chain::iterator::operator++(int) const {
      auto it = *this;
      ++it;
      return it;
   }
   chain::iterator& chain::iterator::operator+=(size_t n) {
      while (n--)
         this->operator++();
      return *this;
   }
   chain::iterator chain::iterator::operator+(size_t n) const {
      auto it = *this;
      it += n;
      return it;
   }
   
   //
   // chain
   //
   
   chain::chain(node n) : _node(n) {}
   chain::chain(tree n) : _node(n) {}
   
   chain::iterator chain::begin() {
      return iterator(this->_node);
   }
   chain::iterator chain::end() {
      return iterator(nullptr);
   }
   chain::iterator chain::at(size_t n) {
      return this->begin() + n;
   }
   
   size_t chain::size() const {
      return list_length(this->_node.unwrap());
   }
   
   node chain::front() {
      return *this->_node;
   }
   node chain::back() {
      return node::wrap(tree_last(this->_node.unwrap()));
   }
   node chain::operator[](size_t n) {
      if (!this->_node)
         throw std::out_of_range("out-of-bounds indexed access to empty `chain`");
      
      size_t i = 0;
      for(auto item = this->_node; item; item = TREE_CHAIN(item.unwrap())) {
         if (i == n)
            return *item;
      }
      throw std::out_of_range("out-of-bounds indexed access to `chain`");
   }
   
   bool chain::contains(node t) const {
      return chain_member(t.unwrap(), this->_node.unwrap());
   }
   
   //
   // Functions for mutating a chain:
   //
   
   void chain::concat(chain other) {
      this->_node = chainon(this->_node.unwrap(), other._node.unwrap());
   }
   void chain::reverse() {
      this->_node = nreverse(this->_node.unwrap());
   }
}