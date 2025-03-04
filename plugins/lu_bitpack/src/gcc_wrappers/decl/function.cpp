#include "gcc_wrappers/decl/function.h"
#include "gcc_wrappers/decl/param.h"
#include "gcc_wrappers/_node_boilerplate-impl.define.h"
#include <cassert>
#include <stdexcept>
#include "lu/stringf.h"
#include <function.h>
#include <stringpool.h> // get_identifier
#include <output.h> // assemble_external
#include <toplev.h>
#include <c-family/c-common.h> // lookup_name, pushdecl, etc.
#include <cgraph.h>
#include <c-tree.h> // c_bind, pushdecl

// function
namespace gcc_wrappers::decl {
   GCC_NODE_WRAPPER_BOILERPLATE(function)
   
   void function::_make_decl_arguments_from_type() {
      auto args = this->function_type().arguments();
      if (!args)
         return;
      
      size_t i    = 0;
      tree   prev = NULL_TREE;
      for(auto pair : *args) {
         if (!pair.second) {
            break;
         }
         if (TREE_CODE(pair.second.unwrap()) == VOID_TYPE) {
            //
            // Sentinel indicating that this is not a varargs function.
            //
            break;
         }
         
         // Generate a name for the arguments, to avoid crashes on a 
         // null IDENTIFIER_NODE when dumping function info elsewhere.
         auto arg_name = lu::stringf("__arg_%u", i++);
         
         auto p = param::wrap(build_decl(
            UNKNOWN_LOCATION,
            PARM_DECL,
            get_identifier(arg_name.c_str()),
            pair.second.unwrap()
         ));
         DECL_ARG_TYPE(p.unwrap()) = pair.second.unwrap();
         DECL_CONTEXT(p.unwrap()) = this->_node;
         if (prev == NULL_TREE) {
            DECL_ARGUMENTS(this->_node) = p.unwrap();
         } else {
            TREE_CHAIN(prev) = p.unwrap();
         }
         prev = p.unwrap();
      }
   }
   
   function::function(lu::strings::zview name, type::function function_type) {
      //
      // Create the node with GCC's defaults (defined elsewhere, extern, 
      // artificial, nothrow).
      //
      this->_node = build_fn_decl(name.c_str(), function_type.unwrap());
      //
      // NOTE: That doesn't automatically build DECL_ARGUMENTS. We need 
      // to do so manually.
      //
      this->_make_decl_arguments_from_type();
   }
   
   type::function function::function_type() const {
      return type::function::wrap(TREE_TYPE(this->_node));
   }
   param function::nth_parameter(size_t n) const {
      size_t i    = 0;
      tree   decl = DECL_ARGUMENTS(this->_node);
      for(; decl != NULL_TREE; decl = TREE_CHAIN(decl)) {
         if (i == n)
            break;
         ++i;
      }
      if (i == n)
         return param::wrap(decl);
      throw std::out_of_range("out-of-bounds function_decl parameter access");
   }
   
   result function::result_variable() const {
      assert(this->has_body());
      return result::wrap(DECL_RESULT(this->_node));
   }

   bool function::is_versioned() const {
      return DECL_FUNCTION_VERSIONED(this->_node);
   }

   bool function::is_always_emitted() const {
      return DECL_PRESERVE_P(this->_node);
   }
   void function::make_always_emitted() {
      set_is_always_emitted(true);
   }
   void function::set_is_always_emitted(bool) {
      DECL_PRESERVE_P(this->_node) = 1;
   }
   
   bool function::is_defined_elsewhere() const {
      return DECL_EXTERNAL(this->_node);
   }
   void function::set_is_defined_elsewhere(bool v) {
      DECL_EXTERNAL(this->_node) = v ? 1 : 0;
   }
            
   bool function::is_externally_accessible() const {
      return TREE_PUBLIC(this->_node);
   }
   void function::make_externally_accessible() {
      set_is_externally_accessible(true);
   }
   void function::set_is_externally_accessible(bool v) {
      TREE_PUBLIC(this->_node) = v ? 1 : 0;
   }

   bool function::is_noreturn() const {
      return TREE_THIS_VOLATILE(this->_node);
   }
   void function::make_noreturn() {
      set_is_noreturn(true);
   }
   void function::set_is_noreturn(bool v) {
      TREE_THIS_VOLATILE(this->_node) = v ? 1 : 0;
   }
   
   bool function::is_nothrow() const {
      return TREE_NOTHROW(this->_node);
   }
   void function::make_nothrow() {
      set_is_nothrow(true);
   }
   void function::set_is_nothrow(bool v) {
      TREE_NOTHROW(this->_node) = v ? 1 : 0;
   }
   
   bool function::has_body() const {
      return DECL_SAVED_TREE(this->_node) != NULL_TREE;
   }
   
   function_with_modifiable_body function::as_modifiable() {
      assert(!has_body());
      {
         auto args = DECL_ARGUMENTS(this->_node);
         if (args == NULL_TREE) {
            this->_make_decl_arguments_from_type();
         } else {
            //
            // Update contexts. In the case of us taking a forward-declared 
            // function and generating a definition for it, the PARM_DECLs 
            // may exist but lack the appropriate DECL_CONTEXT.
            //
            for(; args != NULL_TREE; args = TREE_CHAIN(args)) {
               DECL_CONTEXT(args) = this->_node;
            }
         }
      }
      return function_with_modifiable_body(this->_node);
   }
   
   void function::introduce_to_current_scope() {
      auto prior = lookup_name(DECL_NAME(this->unwrap()));
      if (prior == NULL_TREE) {
         pushdecl(this->unwrap());
      }
   }
   void function::introduce_to_global_scope() {
      
      // `c_bind` is meant for declarations, not definitions, AFAICT anyway. 
      // it therefore does some things we maybe don't want
      auto was_extern = DECL_EXTERNAL(this->_node);
      c_bind(DECL_SOURCE_LOCATION(this->_node), this->_node, true); // c/c-tree.h
      DECL_EXTERNAL(this->_node) = was_extern;
      
      rest_of_decl_compilation(this->_node, true, false); // toplev.h
   }
}

// Avoid warnings on later includes:
#include "gcc_wrappers/_node_boilerplate.undef.h"

#include "gcc_wrappers/decl/result.h"
#include "gcc_wrappers/expr/local_block.h"
#include <tree-iterator.h>

// function_with_modifiable_body
namespace gcc_wrappers::decl {
   function_with_modifiable_body::function_with_modifiable_body(tree n)
      :
      function(_modifiable_subconstruct_tag{})
   {
      this->_node = n;
      assert(raw_node_is(n));
   }
   
   void function_with_modifiable_body::set_result_decl(result decl) {
      DECL_RESULT(this->_node) = decl.unwrap();
      DECL_CONTEXT(decl.unwrap()) = this->_node;
   }
   
   void function_with_modifiable_body::set_root_block(expr::local_block& lb) {
      //
      // The contents of a function are stored in two ways:
      //
      //  - `DECL_SAVED_TREE(func_decl)` is a tree of expressions representing the 
      //    code inside of the function: variable declarations, statements, and so 
      //    on. In this tree, blocks are represented as `BIND_EXPR` nodes.
      //
      //  - `DECL_INITIAL(func_decl)` is a tree of scopes. Each scope is represented
      //    as a `BLOCK` node.
      //
      //    Properties include:
      //
      //       BLOCK_VARS(block)
      //          The first *_DECL node scoped to this block. The TREE_CHAIN of this 
      //          decl is the next decl scoped to this block; and onward.
      //       
      //       BLOCK_SUBBLOCKS(block)
      //          The block's first child.
      //       
      //       BLOCK_CHAIN(block)
      //          The block's next sibling.
      //       
      //       BLOCK_SUPERCONTEXT(block)
      //          The parent BLOCK or FUNCTION_DECL node.
      //
      gcc_assert(BIND_EXPR_BLOCK(lb.unwrap()) == NULL_TREE && "This local block seems to already be in use elsewhere.");
      
      DECL_SAVED_TREE(this->_node) = lb.unwrap();
      TREE_STATIC(this->_node) = 1;
      
      DECL_CONTEXT(DECL_RESULT(this->_node)) = this->_node;
      
      //
      // We need to create the BLOCK hierarchy, and set DECL_CONTEXTs and 
      // similar as appropriate on local declarations within this function.
      //
      
      tree _root_ut = lb.unwrap();
      //
      struct block_to_be {
         tree bind_expr_node = NULL_TREE;
         tree block_node     = NULL_TREE;
      };
      std::vector<block_to_be> seen_blocks;
      //
      // First, find all child and descendant BIND_EXPRs in the function, 
      // and create BLOCKs for them.
      //
      walk_tree(
         &_root_ut,
         [](tree* current_ptr, int* walk_subtrees, void* data) -> tree {
            auto& seen_blocks = *(std::vector<block_to_be>*)data;
            if (TREE_CODE(*current_ptr) == BIND_EXPR) {
               auto& entry = seen_blocks.emplace_back();
               entry.bind_expr_node = *current_ptr;
               entry.block_node     = make_node(BLOCK);
               BIND_EXPR_BLOCK(entry.bind_expr_node) = entry.block_node;
            }
            return NULL_TREE; // if non-null, stops iteration
         },
         &seen_blocks,
         nullptr
      );
      assert(!seen_blocks.empty());
      assert(seen_blocks.front().bind_expr_node == lb.unwrap());
      //
      // Next, walk the subtrees for each local block, but not sub-sub-trees. 
      // That is: if local block A contains local block B, then when we walk 
      // the subtree of A, we walk all nodes that are inside of A but not 
      // also inside of B.
      //
      // Here, we will:
      //
      //  - Link blocks to their parents and siblings.
      //
      //  - Link VAR_DECLs to their containing function and containing BLOCK.
      //
      //  - Link LABEL_DECLs to their containing function.
      //
      for(auto& seen : seen_blocks) {
         struct _inner_link_state {
            optional_function containing_function;
            //
            tree parent_bind_expr    = NULL_TREE;
            tree prev_child_block    = NULL_TREE;
            tree prev_variable_decl  = NULL_TREE;
         } state;
         state.containing_function = *this;
         state.parent_bind_expr    = seen.bind_expr_node;
         
         walk_tree(
            &state.parent_bind_expr,
            [](tree* current_ptr, int* walk_subtrees, void* data) -> tree {
               auto& state   = *(_inner_link_state*)data;
               tree  current = *current_ptr;
                  
               if (current == state.parent_bind_expr)
                  return NULL_TREE;
               
               auto parent_block = BIND_EXPR_BLOCK(state.parent_bind_expr);
               assert(parent_block != NULL_TREE);
               assert(TREE_CODE(parent_block) == BLOCK);
               
               // Set up hierarchical relationships between BLOCKs.
               if (TREE_CODE(current) == BIND_EXPR) {
                  // Don't walk the contents of a child or descendant BIND_EXPR. 
                  // A future iteration of the per-seen-block loop should hit 
                  // the BIND_EXPR and deal with its own sub-tree.
                  *walk_subtrees = 0;
                  
                  auto current_block = BIND_EXPR_BLOCK(current);
                  assert(current_block != NULL_TREE);
                  assert(TREE_CODE(current_block) == BLOCK);
                  //
                  BLOCK_SUPERCONTEXT(current_block) = parent_block;
                  if (state.prev_child_block != NULL_TREE) {
                     BLOCK_CHAIN(state.prev_child_block) = current_block;
                  } else {
                     BLOCK_SUBBLOCKS(parent_block) = current_block;
                  }
                  state.prev_child_block = current_block;
                  return NULL_TREE;
               }
               
               // Parent VAR_DECLs to their containing FUNCTION_DECL and BLOCK, 
               // and tie them to their siblings as well.
               if (TREE_CODE(current) == DECL_EXPR) {
                  auto decl = DECL_EXPR_DECL(current);
                  assert(decl != NULL_TREE && "A DECL_EXPR exists to indicate the location at which a VAR_DECL is declared; this DECL_EXPR must indicate the VAR_DECL that it exists for.");
                  assert(DECL_P(decl) && "A DECL_EXPR must not be malformed.");
                  if (TREE_CODE(decl) == VAR_DECL) {
                     assert((
                        DECL_CONTEXT(decl) == NULL_TREE ||
                        DECL_CONTEXT(decl) == state.containing_function.unwrap()
                     ) && "This VAR_DECL, tied to a DECL_EXPR in this function, must not already belong to a different function.");
                     DECL_CONTEXT(decl) = state.containing_function.unwrap();
                     
                     assert(TREE_CHAIN(decl) == NULL_TREE && "This VAR_DECL must not already be linked to a BLOCK's variables. If it is, then the same VAR_DECL may have been declared in multiple functions, or multiple times in the same function, by mistake.");
                     if (state.prev_variable_decl != NULL_TREE) {
                        TREE_CHAIN(state.prev_variable_decl) = decl;
                     } else {
                        BLOCK_VARS(parent_block) = decl;
                        BIND_EXPR_VARS(state.parent_bind_expr) = decl;
                     }
                     state.prev_variable_decl = decl;
                  }
                  return NULL_TREE;
               }
               
               // Parent LABEL_DECLs to their containing FUNCTION_DECL.
               if (TREE_CODE(current) == LABEL_EXPR) {
                  auto decl = LABEL_EXPR_LABEL(current); // LABEL_DECL
                  assert(decl != NULL_TREE && "A LABEL_EXPR exists to indicate the location at which a LABEL_DECL is declared; this LABEL_EXPR must indicate the LABEL_DECL that it exists for.");
                  assert(TREE_CODE(decl) == LABEL_DECL && "A LABEL_EXPR must not be malformed.");
                  assert((
                     DECL_CONTEXT(decl) == NULL_TREE ||
                     DECL_CONTEXT(decl) == state.containing_function.unwrap()
                  ) && "This LABEL_DECL must not already belong to a different function than the one it's currently being used in.");
                  DECL_CONTEXT(decl) = state.containing_function.unwrap();
                  return NULL_TREE;
               }
               
               if (TREE_CODE(current) == BLOCK) {
                  *walk_subtrees = 0;
               }
               
               return NULL_TREE;
            },
            &state,
            nullptr
         );
      }
      //
      // Fix-up for TREE_SIDE_EFFECTS on each BIND_EXPR.
      //
      for(auto& seen : seen_blocks) {
         auto expr = seen.bind_expr_node;
         if (TREE_SIDE_EFFECTS(expr))
            continue;
         walk_tree(
            &expr,
            [](tree* current_ptr, int* walk_subtrees, void* data) -> tree {
               tree walking_from = *(tree*)data;
               tree walking_at   = *current_ptr;
               if (walking_from == walking_at)
                  return NULL_TREE;
               
               if (TREE_SIDE_EFFECTS(walking_at)) {
                  TREE_SIDE_EFFECTS(walking_from) = 1;
                  return walking_from; // stop iteration by returning non-null
               }
               
               return NULL_TREE;
            },
            &expr,
            nullptr
         );
      }
      //
      // Finally, take the root block and link it to this function.
      //
      DECL_INITIAL(this->_node) = seen_blocks.front().block_node;
      BLOCK_SUPERCONTEXT(seen_blocks.front().block_node) = this->_node;
      
      #ifndef NDEBUG
      {
         tree fd  = this->unwrap();
         tree lbn = lb.unwrap();
         walk_tree(
            &lbn,
            [](tree* current_ptr, int* walk_subtrees, void* data) -> tree {
               tree function   = *(tree*)data;
               tree walking_at = *current_ptr;
               assert(TREE_CODE(function) == FUNCTION_DECL);
               if (TREE_CODE(walking_at) == LABEL_EXPR) {
                  auto decl = LABEL_EXPR_LABEL(walking_at);
                  assert(DECL_CONTEXT(decl) == function);
               }
               return NULL_TREE;
            },
            &fd,
            nullptr
         );
      }
      #endif
      
      //
      // Update contexts. In the case of us taking a forward-declared 
      // function and generating a definition for it, the PARM_DECLs 
      // may exist but lack the appropriate DECL_CONTEXT.
      //
      for(auto args = DECL_ARGUMENTS(this->_node); args != NULL_TREE; args = TREE_CHAIN(args)) {
         DECL_CONTEXT(args) = this->_node;
      }
      
      //
      // Actions below are needed in order to emit the function to the object file, 
      // so the linker can see it and it's included in the program.
      //
      // NOTE: You must also mark the function as non-extern, manually.
      //
      
      if (!DECL_STRUCT_FUNCTION(this->_node)) {
         //
         // We need to allocate a "struct function" (i.e. a struct that describes 
         // some stuff about the function) in order for the function to be truly 
         // complete. (Unclear on what purpose this serves exactly, but I do know 
         // that the GCC functions to debug-print a whole FUNCTION_DECL won't even 
         // attempt to print the PARM_DECLs unless a struct function is present.) 
         //
         // Allocating a struct function triggers "layout" of the FUNCTION_DECL 
         // and its associated DECLs, including the RESULT_DECL. As such, if there 
         // is no RESULT_DECL, then GCC crashes.
         //
         gcc_assert(DECL_RESULT(this->_node) != NULL_TREE);
         allocate_struct_function(this->_node, false); // function.h
      }
      
      // per `finish_function` in `c/c-decl.cc`:
      if (!decl_function_context(this->_node)) {
         // re: our issue with crashing when trying to get the per-sector functions 
         // into the object file: this branch doesn't make a difference in that 
         // regard.
         if constexpr (false) {
            cgraph_node::add_new_function(this->_node, false);
         } else {
            cgraph_node::finalize_function(this->_node, false); // cgraph.h
         }
      } else {
         //
         // We're a nested function.
         //
         cgraph_node::get_create(this->_node);
      }
   }
}