#include <stdexcept>
#include "gcc_wrappers/attribute_list.h"
#include "gcc_wrappers/attribute.h"
#include <stringpool.h> // dependency for <attribs.h>
#include <attribs.h>

namespace gcc_wrappers {
   
   //
   // attribute_list::iterator
   //
   
   attribute_list::iterator::iterator(optional_node n) : _node(n) {}
   
   attribute attribute_list::iterator::operator*() {
      return attribute::wrap(this->_node.unwrap());
   }

   attribute_list::iterator& attribute_list::iterator::operator++() {
      assert(this->_node);
      this->_node = TREE_CHAIN(this->_node.unwrap());
      return *this;
   }
   attribute_list::iterator attribute_list::iterator::operator++(int) const {
      auto it = *this;
      ++it;
      return it;
   }
   
   //
   // attribute_list
   //
   
   bool attribute_list::operator==(const attribute_list other) const noexcept {
      if (this->empty())
         return other.empty();
      else if (other.empty())
         return false;
      return attribute_list_equal(this->unwrap(), other.unwrap());
   }
   
   bool attribute_list::contains(const attribute attr) const {
      if (empty())
         return false;
      return attribute_list_contained(this->_head->unwrap(), attr.unwrap());
   }
   bool attribute_list::empty() const {
      return !this->_head;
   }
   
   attribute_list::iterator attribute_list::begin() {
      return iterator(this->_head);
   }
   attribute_list::iterator attribute_list::end() {
      return iterator(NULL_TREE);
   }
   attribute_list::iterator attribute_list::at(size_t n) {
      if (!this->_head)
         throw std::out_of_range("out-of-bounds access to an attribute's argument expressions");
      
      tree raw = this->_head.unwrap();
      for(size_t i = 0; i < n; ++i) {
         raw = TREE_CHAIN(raw);
         if (raw == NULL_TREE)
            throw std::out_of_range("out-of-bounds access to an attribute's argument expressions");
      }
      return iterator(raw);
   }
   
   optional_attribute attribute_list::first_attribute_with_prefix(lu::strings::zview prefix) {
      return std::as_const(*this).first_attribute_with_prefix(prefix);
   }
   const optional_attribute attribute_list::first_attribute_with_prefix(lu::strings::zview prefix) const {
      return lookup_attribute_by_prefix(prefix.c_str(), this->unwrap());
   }
   
   optional_attribute attribute_list::get_attribute(lu::strings::zview name) {
      return std::as_const(*this).get_attribute(name);
   }
   const optional_attribute attribute_list::get_attribute(lu::strings::zview name) const {
      return lookup_attribute(name.c_str(), this->unwrap());
   }
   
   bool attribute_list::has_attribute(lu::strings::zview name) const {
      if (empty())
         return false;
      auto attr = this->get_attribute(name);
      return (bool)attr;
   }
   bool attribute_list::has_attribute(tree id_node) const {
      assert(id_node != NULL_TREE);
      assert(TREE_CODE(id_node) == IDENTIFIER_NODE);
      if (empty())
         return false;
      return has_attribute(IDENTIFIER_POINTER(id_node));
   }
         
   void attribute_list::remove_attribute(lu::strings::zview name) {
      if (empty())
         return;
      this->_head = ::remove_attribute(name.c_str(), this->unwrap());
   }
}