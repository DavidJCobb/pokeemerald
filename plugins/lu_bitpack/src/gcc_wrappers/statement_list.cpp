#include "gcc_wrappers/statement_list.h"
#include "gcc_wrappers/expr/base.h"
#include <cassert>
#include "gcc_wrappers/_node_boilerplate-impl.define.h"

namespace gcc_wrappers {
   GCC_NODE_WRAPPER_BOILERPLATE(statement_list)
   
   using iterator_type = statement_list::iterator;
   
   //
   // statement_list::iterator
   //

   node iterator_type::operator*() {
      return node::wrap(tsi_stmt(this->_data));
   }

   iterator_type& iterator_type::operator--() {
      tsi_prev(&this->_data);
      return *this;
   }
   iterator_type iterator_type::operator--(int) {
      iterator_type it(*this);
      return --it;
   }
   iterator_type& iterator_type::operator++() {
      assert(!tsi_end_p(this->_data));
      tsi_next(&this->_data);
      return *this;
   }
   iterator_type iterator_type::operator++(int) {
      iterator_type it(*this);
      return ++it;
   }
   
   //
   // statement_list
   //
   
   void statement_list::_require_not_moved_from() const {
      assert(this->_node != NULL_TREE);
   }
   void statement_list::_append(tree t) {
      auto it = tsi_last(this->_node);
      tsi_link_after(&it, t, TSI_CONTINUE_LINKING);
   }
   void statement_list::_prepend(tree t) {
      _require_not_moved_from();
      auto it = tsi_start(this->_node);
      tsi_link_after(&it, t, TSI_CONTINUE_LINKING);
   }
   
   statement_list::statement_list() {
      this->_node = alloc_stmt_list();
   }
   
   statement_list::iterator statement_list::begin() {
      _require_not_moved_from();
      iterator it;
      it._data = tsi_start(this->_node);
      return it;
   }
   statement_list::iterator statement_list::end() {
      _require_not_moved_from();
      iterator it;
      it._data     = tsi_start(this->_node);
      it._data.ptr = nullptr;
      return it;
   }
   
   bool statement_list::empty() const {
      _require_not_moved_from();
      return STATEMENT_LIST_HEAD(this->_node) == nullptr;
   }
   
   void statement_list::append(const expr::base& expr) {
      _append((tree)expr.unwrap());
   }
   void statement_list::append(statement_list&& other) {
      _append((tree)other.unwrap());;
      other._node = NULL_TREE;
   }
   void statement_list::prepend(const expr::base& expr) {
      _prepend((tree)expr.unwrap());
   }
   void statement_list::prepend(statement_list&& other) {
      _prepend((tree)other.unwrap());
      other._node = NULL_TREE;
   }
   
   void statement_list::append_untyped(node expr) {
      _require_not_moved_from();
      _append((tree)expr.unwrap());
   }
}