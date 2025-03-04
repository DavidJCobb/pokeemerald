#pragma once
#include "lu/strings/zview.h"
#include "gcc_wrappers/decl/base_value.h"
#include "gcc_wrappers/expr/declare.h"
#include "gcc_wrappers/type/base.h"
#include "gcc_wrappers/value.h"
#include "gcc_wrappers/_node_boilerplate.define.h"

namespace gcc_wrappers {
   class identifier;
}

namespace gcc_wrappers::decl {
   class variable : public base_value {
      public:
         static bool raw_node_is(tree t) {
            return TREE_CODE(t) == VAR_DECL;
         }
         GCC_NODE_WRAPPER_BOILERPLATE(variable)
         
      public:
         variable(
            lu::strings::zview identifier_name,
            type::base         value_type,
            location_t         source_location = UNKNOWN_LOCATION
         );
         variable(
            identifier identifier_node,
            type::base value_type,
            location_t source_location = UNKNOWN_LOCATION
         );
      
         optional_value initial_value() const;
         void set_initial_value(optional_value);
         
         expr::declare make_declare_expr();
         
         bool is_defined_elsewhere() const;
         void set_is_defined_elsewhere(bool);
         
         // Indicates that this decl's name is to be accessible from 
         // outside of the current translation unit.
         bool is_externally_accessible() const; // TREE_PUBLIC
         void make_externally_accessible();
         void set_is_externally_accessible(bool);
         
         // Indicates whether the variable has ever been used in any 
         // manner other than having its value set.
         bool is_ever_read() const;
         void make_ever_read();
         void set_is_ever_read(bool);
         
         // The getter should always be safe.
         //
         // The setter makes this assertion unconditionally:
         //    assert(gcc_wrappers::environment::c::constexpr_supported);
         //
         // The setter makes this assertion if you try to mark the 
         // variable as declared-constexpr, but not if you try to 
         // clear that flag:
         //    assert(gcc_wrappers::environment::c::current_dialect() >= ...::c23);
         bool is_declared_constexpr() const;
         void make_declared_constexpr();
         void set_is_declared_constexpr(bool);
         
         
         // Attempt to introduce this variable into the file/global scope, 
         // such that name lookups, etc., can find it; and finalizes 
         // compilation of the variable, generating whatever assembler 
         // labels, etc., are necessary.
         void make_file_scope_extern();
         
         // Attempt to introduce this variable into the scope that the 
         // parser is currently in, such that name lookups, etc., can 
         // find the variable; and finalizes compilation of the variable, 
         // generating whatever assembler labels, etc., are necessary.
         void commit_to_current_scope();
   };
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(variable);
}

#include "gcc_wrappers/_node_boilerplate.undef.h"