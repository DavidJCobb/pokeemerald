#include "gcc_wrappers/expr/local_block.h"
#include "gcc_wrappers/_node_boilerplate-impl.define.h"

namespace gcc_wrappers::expr {
   GCC_NODE_WRAPPER_BOILERPLATE(local_block)
   
   local_block::local_block() {
      this->_node = build3(
         BIND_EXPR,
         void_type_node,    // return type
         NULL,              // vars (accessible via BIND_EXPR_VARS(expr))
         alloc_stmt_list(), // STATEMENT_LIST node
         NULL_TREE          // BLOCK node
      );
   }
   local_block::local_block(type::base desired) {
      this->_node = build3(
         BIND_EXPR,
         desired.unwrap(), // return type
         NULL,                 // vars (accessible via BIND_EXPR_VARS(expr))
         alloc_stmt_list(),    // STATEMENT_LIST node
         NULL_TREE             // BLOCK node
      );
   }
   local_block::local_block(statement_list&& statements) {
      this->_node = build3(
         BIND_EXPR,
         void_type_node,          // return type
         NULL,                    // vars (accessible via BIND_EXPR_VARS(expr))
         statements.unwrap(), // STATEMENT_LIST node
         NULL_TREE                // BLOCK node
      );
   }
   local_block::local_block(type::base desired, statement_list&& statements) {
      this->_node = build3(
         BIND_EXPR,
         desired.unwrap(),    // return type
         NULL,                // vars (accessible via BIND_EXPR_VARS(expr))
         statements.unwrap(), // STATEMENT_LIST node
         NULL_TREE            // BLOCK node
      );
   }
   
   statement_list local_block::statements() {
      return statement_list::wrap(BIND_EXPR_BODY(this->_node));
   }
}