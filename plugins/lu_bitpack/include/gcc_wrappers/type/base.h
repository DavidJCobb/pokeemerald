#pragma once
#include <string>
#include "gcc_wrappers/node.h"
#include "gcc_wrappers/optional.h"
#include "gcc_wrappers/_node_boilerplate.define.h"

namespace gcc_wrappers {
   namespace decl {
      class type_def;
      DECLARE_GCC_OPTIONAL_NODE_WRAPPER(type_def);
   }
   namespace type {
      class array;
      class container;
      class enumeration;
      class fixed_point;
      class floating_point;
      class function;
      class integral;
      class pointer;
      class record;
      class untagged_union;
   }
   class attribute_list;
}

namespace gcc_wrappers::type {
   struct assign_check_options {
      bool as_builtin_argument          = false;
      bool as_function_argument         = false;
      bool require_cpp_compat           = false;
      bool rhs_is_null_pointer_constant = false;
   };
   
   class base : public node {
      public:
         static bool raw_node_is(tree t) {
            return TYPE_P(t);
         }
         GCC_NODE_WRAPPER_BOILERPLATE(base)
         
      public:
         // Equality comparison. Use `is_same` for identity.
         bool operator==(const base) const;
         bool is_equal(const base o) const {
            return operator==(o);
         }
         
         std::string name() const;
         std::string tag_name() const;
         std::string pretty_print() const;
            
         // The source location of the TYPE_DECL which declared this type, if any.
         location_t source_location() const;
         
         // NOTE: This only includes attributes that GCC has decided to apply directly 
         // to the type (generally done when it's a struct or union). It notably does 
         // not include attributes applied to a `typedef`; those exist on the TYPE_DECL, 
         // not on the *_TYPE that the TYPE_DECL creates.
         attribute_list attributes();
         const attribute_list attributes() const;
         
         base canonical() const;
         base main_variant() const;
         
         // If this type was defined via the `typedef` keyword, returns the TYPE_DECL 
         // produced by that keyword. You can walk transitive typedefs this way. If 
         // this type isn't the result of a `typedef`, then the result is `empty()`.
         //
         // Make sure to include the header for `type_def` yourself before calling. 
         // Can't include it for you here or we'll get a circular dependency.
         decl::optional_type_def declaration() const;
         
         bool is_type_or_transitive_typedef_thereof(base) const;
         
         bool has_user_requested_alignment() const; // __attribute__((align...))
         bool is_complete() const;
         bool is_packed() const;
         
         size_t alignment_in_bits() const;
         size_t alignment_in_bytes() const; // rounds down
         size_t minimum_alignment_in_bits() const; // GCC warns if the type isn't aligned to this threshold
         size_t size_in_bits() const;
         size_t size_in_bytes() const;
         
         //
         // Type checks:
         //
         
         bool is_arithmetic() const; // INTEGER_TYPE or REAL_TYPE
         bool is_array() const;
         bool is_bitint() const; // BITINT_TYPE
         bool is_boolean() const;
         bool is_complex() const; // COMPLEX_TYPE
         bool is_container() const;
         bool is_enum() const; // ENUMERAL_TYPE
         bool is_fixed_point() const;
         bool is_floating_point() const; // REAL_TYPE
         bool is_function() const; // FUNCTION_TYPE or METHOD_TYPE
         bool is_integer() const; // does not include enums or bools
         bool is_integral() const {
            return is_boolean() || is_enum() || is_integer();
         }
         bool is_method() const; // METHOD_TYPE
         bool is_null_pointer_type() const; // NULLPTR_TYPE i.e. decltype(nullptr)
         bool is_record() const; // class or struct
         bool is_union() const;
         bool is_vector() const; // VECTOR_TYPE
         bool is_void() const;
         
         // Check whether something is a BOOLEAN_TYPE or an enum whose underlying 
         // type is a bool.
         bool has_c_boolean_semantics() const;
         
         //
         // Casts:
         //
         
         array as_array();
         const array as_array() const;
         
         container as_container();
         const container as_container() const;
         
         enumeration as_enum();
         const enumeration as_enum() const;
         
         floating_point as_floating_point();
         const floating_point as_floating_point() const;
         
         fixed_point as_fixed_point();
         const fixed_point as_fixed_point() const;
         
         function as_function();
         const function as_function() const;
         
         integral as_integral();
         const integral as_integral() const;
         
         pointer as_pointer();
         const pointer as_pointer() const;
         
         record as_record();
         const record as_record() const;
         
         untagged_union as_union();
         const untagged_union as_union() const;
         
         //
         // Queries and transformations:
         //
         
      protected:
         base _with_qualifiers_added(int) const;
         base _with_qualifiers_removed(int) const;
         base _with_qualifiers_replaced(int) const;
         
      public:
         base with_all_qualifiers_stripped() const;
      
         bool is_atomic() const;
         base add_atomic() const;
         base remove_atomic() const;
      
         base add_const() const;
         base remove_const() const;
         bool is_const() const;
         
         bool is_volatile() const;
         base add_volatile() const;
         base remove_volatile() const;
         
         pointer add_pointer() const;
         base remove_pointer() const;
         bool is_pointer() const;
         
         bool is_reference() const;
         bool is_lvalue_reference() const;
         bool is_rvalue_reference() const;
         
         bool is_restrict() const;
         bool is_restrict_allowed() const;
         base add_restrict() const; // assert(is_restrict_allowed());
         base remove_restrict() const;
         
         array add_array_extent(size_t) const;
         
         // assert(!is_packed())
         // If this type already has the desired alignment (whether user-requested 
         // or not), returns self.
         base with_user_defined_alignment(size_t bytes);
         base with_user_defined_alignment_in_bits(size_t bits);
         
         //
         // Tests:
         //
         
         // Check if values of type `this` are assignable to variables of type `lhs` 
         // without an explicit cast. This can also be used to check whther values 
         // of type `this` can be passed to function arguments whose declared types 
         // are `lhs`, provided you set the right options for running the check.
         //
         // NOTE: Untested.
         bool is_assignable_to(base lhs, const assign_check_options& = {}) const;
   };
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(base);
}

#include "gcc_wrappers/_node_boilerplate.undef.h"