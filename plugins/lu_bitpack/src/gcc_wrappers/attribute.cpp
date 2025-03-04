#include "gcc_wrappers/attribute.h"
#include "gcc_wrappers/identifier.h"
#include <stdexcept>
#include <stringpool.h> // get_identifier, and dependency for <attribs.h>
#include <attribs.h>
#include "gcc_wrappers/_node_boilerplate-impl.define.h"

namespace gcc_wrappers {
   GCC_NODE_WRAPPER_BOILERPLATE(attribute)
   
   //
   // attribute::arguments_wrapper::iterator
   //
   
   attribute::arguments_wrapper::iterator::iterator(tree t) : _node(t) {}
   
   node attribute::arguments_wrapper::iterator::operator*() {
      assert(this->_node != NULL_TREE);
      auto arg_value_node = TREE_VALUE(this->_node);
      
      return node::wrap(arg_value_node);
   }

   attribute::arguments_wrapper::iterator& attribute::arguments_wrapper::iterator::operator++() {
      assert(this->_node != NULL_TREE);
      this->_node = TREE_CHAIN(this->_node);
      return *this;
   }
   attribute::arguments_wrapper::iterator attribute::arguments_wrapper::iterator::operator++(int) const {
      auto it = *this;
      ++it;
      return it;
   }
   
   //
   // attribute::arguments_wrapper
   //
   
   attribute::arguments_wrapper::arguments_wrapper(tree t) : _node(t) {
   }
   
   bool attribute::arguments_wrapper::empty() const noexcept {
      return this->_node == NULL_TREE;
   }

   size_t attribute::arguments_wrapper::size() const noexcept {
      auto   raw = this->_node;
      size_t i    = 0;
      while (raw != NULL_TREE) {
         ++i;
         raw = TREE_CHAIN(raw);
      }
      return i;
   }
   node attribute::arguments_wrapper::operator[](size_t n) {
      auto raw = this->_node;
      if (raw == NULL_TREE)
         throw std::out_of_range("out-of-range access to an attribute's arguments");
      
      for(size_t i = 0; i < n; ++i) {
         raw = TREE_CHAIN(raw);
         if (raw == NULL_TREE)
            throw std::out_of_range("out-of-range access to an attribute's arguments");
      }
      return node::wrap(TREE_VALUE(raw));
   }
   
   node attribute::arguments_wrapper::front() {
      if (empty())
         throw std::out_of_range("out-of-range access to an attribute's arguments");
      return *begin();
   }
   node attribute::arguments_wrapper::back() {
      return std::as_const(*this).back();
   }
   const node attribute::arguments_wrapper::front() const {
      return const_cast<arguments_wrapper&>(*this).front();
   }
   const node attribute::arguments_wrapper::back() const {
      if (empty())
         throw std::out_of_range("out-of-range access to an attribute's arguments");
      auto raw = this->_node;
      if (raw != NULL_TREE) {
         while (TREE_CHAIN(raw) != NULL_TREE) {
            raw = TREE_CHAIN(raw);
         }
      }
      return node::wrap(TREE_VALUE(raw));
   }

   attribute::arguments_wrapper::iterator attribute::arguments_wrapper::begin() {
      return iterator(this->_node);
   }
   attribute::arguments_wrapper::iterator attribute::arguments_wrapper::end() {
      return iterator(NULL_TREE);
   }
   attribute::arguments_wrapper::iterator attribute::arguments_wrapper::at(size_t n) {
      auto node = this->_node;
      assert(node != NULL_TREE);
      if (node == NULL_TREE)
         throw std::out_of_range("out-of-bounds access to an attribute's argument expressions");
      
      for(size_t i = 0; i < n; ++i) {
         node = TREE_CHAIN(node);
         if (node == NULL_TREE)
            throw std::out_of_range("out-of-bounds access to an attribute's argument expressions");
      }
      return iterator(node);
   }

   //
   // attribute
   //
   
   attribute::attribute(identifier name, optional_list_node args) {
      this->_node = tree_cons(
         name.unwrap(),
         args.unwrap(),
         NULL_TREE
      );
   }
   attribute::attribute(lu::strings::zview name, optional_list_node args) {
      this->_node = tree_cons(
         get_identifier(name.c_str()),
         args.unwrap(),
         NULL_TREE
      );
   }
   
   std::string_view attribute::name() const {
      auto id_node = get_attribute_name(this->_node);
      if (TREE_CODE(id_node) != IDENTIFIER_NODE)
         return {};
      return IDENTIFIER_POINTER(id_node);
   }
   std::string_view attribute::namespace_name() const {
      auto id_node = get_attribute_namespace(this->_node);
      if (TREE_CODE(id_node) != IDENTIFIER_NODE)
         return {};
      return IDENTIFIER_POINTER(id_node);
   }
   
   optional_identifier attribute::name_node() const {
      auto raw = get_attribute_name(this->_node);
      if (raw == NULL_TREE || identifier::raw_node_is(raw))
         return raw;
      return {};
   }
   optional_identifier attribute::namespace_name_node() const {
      auto raw = get_attribute_namespace(this->_node);
      if (raw == NULL_TREE || identifier::raw_node_is(raw))
         return raw;
      return {};
   }
   
   bool attribute::is_cpp11() const {
      return cxx11_attribute_p(this->_node);
   }
   
   bool attribute::compare_values(const attribute other) const {
      return attribute_value_equal(this->unwrap(), other.unwrap());
   }
   
   attribute::arguments_wrapper attribute::arguments() {
      return std::as_const(*this).arguments();
   }
   const attribute::arguments_wrapper attribute::arguments() const {
      return arguments_wrapper(TREE_VALUE(this->_node));
   }
}