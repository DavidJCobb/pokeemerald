#pragma once
#include "lu/strings/zview.h"
#include "gcc_wrappers/decl/base.h"
#include "gcc_wrappers/decl/param.h"
#include "gcc_wrappers/decl/result.h"
#include "gcc_wrappers/type/function.h"
#include "gcc_wrappers/_node_boilerplate.define.h"

namespace gcc_wrappers {
   namespace decl {
      class result;
   }
   namespace expr {
      class local_block;
   }
}

namespace gcc_wrappers::decl {
   class function_with_modifiable_body;
   
   class function : public base {
      public:
         static bool raw_node_is(tree t) {
            return TREE_CODE(t) == FUNCTION_DECL;
         }
         GCC_NODE_WRAPPER_BOILERPLATE(function)
         
      protected:
         enum class _modifiable_subconstruct_tag {};
         function(_modifiable_subconstruct_tag) {}
         
         void _make_decl_arguments_from_type();
         
      public:
         function(
            lu::strings::zview identifier_name,
            type::function     function_type
         );
         
         type::function function_type() const;
         param nth_parameter(size_t) const;
         
         // assert(this->has_body());
         // If the function is only declared, not defined, then it 
         // should not have a result variable yet.
         result result_variable() const;
         
         bool is_versioned() const; // DECL_FUNCTION_VERSIONED
         
         // DECL_PRESERVE_P
         bool is_always_emitted() const;
         void make_always_emitted();
         void set_is_always_emitted(bool);
         
         // DECL_EXTERNAL
         //
         // NOTE: a C99 "extern inline" function will be marked as 
         // being defined elsewhere yet still have a definition in 
         // the current translation unit.
         bool is_defined_elsewhere() const;
         void set_is_defined_elsewhere(bool);
         
         // Indicates that this decl's name is to be accessible from 
         // outside of the current translation unit.
         bool is_externally_accessible() const; // TREE_PUBLIC
         void make_externally_accessible();
         void set_is_externally_accessible(bool);
         
         bool is_noreturn() const; // TREE_THIS_VOLATILE
         void make_noreturn();
         void set_is_noreturn(bool);
         
         bool is_nothrow() const;
         void make_nothrow();
         void set_is_nothrow(bool);
         
         bool has_body() const;
         
         // assert(!has_body());
         function_with_modifiable_body as_modifiable();
         
         void introduce_to_current_scope(); // wherever the parser happens to be rn
         void introduce_to_global_scope();
   };
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(function);
   
   class function_with_modifiable_body : public function {
      friend class function;
      protected:
         function_with_modifiable_body(tree);
      
      public:
         void set_result_decl(result);
         void set_root_block(expr::local_block&);
   };
}

#include "gcc_wrappers/_node_boilerplate.undef.h"