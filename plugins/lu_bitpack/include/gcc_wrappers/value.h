#pragma once
#include "gcc_wrappers/node.h"
#include "gcc_wrappers/type/base.h"
#include "gcc_wrappers/type/enumeration.h"
#include "gcc_wrappers/type/floating_point.h"
#include "gcc_wrappers/type/integral.h"
#include "gcc_wrappers/enums/round_value_toward.h"
#include "gcc_wrappers/_node_boilerplate.define.h"

namespace gcc_wrappers {
   namespace decl {
      class base_value;
   }
   namespace expr {
      class base;
   }
   
   class value : public node {
      public:
         static bool raw_node_is(tree);
         GCC_NODE_WRAPPER_BOILERPLATE(value)
      
         decl::base_value as_decl() const;
         expr::base as_expr() const;
         
      protected:
         value _make_arith1(tree_code expr_type);
         value _make_arith2(tree_code expr_type, value other);
         value _make_bitwise2(tree_code expr_type, value other);
         value _make_cmp2(tree_code expr_type, value other);
      
      public:
         // Value/result ype.
         type::base value_type() const;
         
         // Creates a COMPONENT_REF.
         // This value must be of a RECORD_TYPE or UNION_TYPE.
         value access_member(const char*);
         
         // Creates a ARRAY_REF.
         // This value must be of an ARRAY_TYPE. Index must be an INTEGER_TYPE.
         value access_array_element(value index);
         
         // Creates a ARRAY_RANGE_REF.
         // This value must be of an ARRAY_TYPE.
         value access_array_range(value start, value length);
         
         // Returns a value of a POINTER_TYPE.
         value address_of();
         
         // `a->b` is `a.deference().access_member("b")`
         // This value must be a POINTER_TYPE or REFERENCE_TYPE.
         value dereference();
         
         // Returns true if the value is a constant value or a constant expression.
         bool is_constant() const;
         
         bool is_lvalue() const;
         
         bool is_associative_operator() const; // a + b + c == (a + b) + c == a + (b + c)
         bool is_commutative_operator() const; // a + b == b + a
         
         //#pragma region Constant integer checks
            // Check if EXPR is a zero-value INTEGER_CST, VECTOR_CST, or 
            // COMPLEX_CST.
            bool is_constant_int_zero() const;
            bool is_constant_int_nonzero() const;
            bool is_constant_int_one() const;
            bool is_constant_int_neg_one() const;
            
            // for VECTOR_CST, this isn't quite the same as being "one."
            bool is_constant_true() const;
            
            // Check if EXPR is an INTEGER_CST, VECTOR_CST, or COMPLEX_CST 
            // for which all defined bits are set.
            bool is_constant_int_all_bits_set() const;
            
            // compare to C++ <bit> functions
            bool constant_has_single_bit() const;
            int constant_bit_floor() const;
         //#pragma endregion
         
         //#pragma region Constant float/real checks
            bool is_constant_real_zero() const;
         //#pragma endregion
         
         //
         // Functions below create expressions, treating `this` as a unary 
         // operand or the lefthand operand (as appropriate). The functions 
         // assert that type requirements are met.
         //
         
         //#pragma region Conversions
            bool convertible_to_enum(type::enumeration) const;
            bool convertible_to_floating_point() const;
            bool convertible_to_integer(type::integral) const;
            bool convertible_to_pointer() const;
            bool convertible_to_vector(type::base) const;
            
            value convert_to_enum(type::enumeration);
            value convert_to_floating_point(type::floating_point);
            value convert_to_integer(type::integral);
            value convert_to_pointer(type::pointer);
            
            // Optimize an expr for use as the condition of an if-statement, 
            // while-statement, ternary, etc..
            value convert_to_truth_value();
            
            // u8[n] -> u8*
            value convert_array_to_pointer();
            
            // Use for:
            //  - Explicitly casting away constness of pointers
            //  - Explicit casts between different pointer types
            value conversion_sans_bytecode(type::base);
            
            value perform_default_promotions();
         //#pragma endregion
         
         //#pragma region Comparison operators
            value cmp_is_less(value);             // LT_EXPR
            value cmp_is_less_or_equal(value);    // LE_EXPR
            value cmp_is_greater(value);          // GT_EXPR
            value cmp_is_greater_or_equal(value); // GE_EXPR
            value cmp_is_equal(value);            // EQ_EXPR
            value cmp_is_not_equal(value);        // NE_EXPR
         //#pragma endregion
         
         // Arithmetic operators. If you try mixing ints and floats, we'll 
         // convert the non-`this` type to match the `this` type.
         //
         // TODO: Overloads taking an integer constant.
         //#pragma region Arithmetic operators
            value arith_abs(); // ABS_EXPR
            value arith_neg(); // NEGATE_EXPR
            
            value complex_conjugate(); // CONJ_EXPR
            
            // Standard increment/decrement operations, applying to integers, 
            // floats, pointers, and complex numbers as is standard.
            value decrement_pre();  // PREDECREMENT_EXPR
            value decrement_post(); // POSTDECREMENT_EXPR
            value increment_pre();  // PREINCREMENT_EXPR
            value increment_post(); // POSTINCREMENT_EXPR
            
            // Non-standard increment/decrement operations wherein you can 
            // choose a delta other than 1. Default conversions are not 
            // performed, and this is not allowed on pointers (because I 
            // can't be bothered to replicate the behaviors, frankly.)
            value decrement_pre(value by);  // PREDECREMENT_EXPR
            value decrement_post(value by); // POSTDECREMENT_EXPR
            value increment_pre(value by);  // PREINCREMENT_EXPR
            value increment_post(value by); // POSTINCREMENT_EXPR
            
            // Arithmetic only. Pointer math is not allowed or performed.
            value add(value);
            value sub(value);
            value mul(value);
            value mod(value); // TRUNC_MOD_EXPR
            
            // Generates different expression types depending on whether `this` is 
            // an integer or a float. Converts the divisor to match.
            value div(value divisor, round_value_toward rounding = round_value_toward::zero);
            
            value min(value);
            value max(value);
         //#pragma endregion
         
         //#pragma region Bitwise operators, available only on integers
            value bitwise_and(value other); // BIT_AND_EXPR
            value bitwise_not(); // BIT_NOT_EXPR
            value bitwise_or(value other);  // BIT_IOR_EXPR
            value bitwise_xor(value other); // BIT_XOR_EXPR
            value shift_left(value bitcount); // LSHIFT_EXPR
            value shift_right(value bitcount); // RSHIFT_EXPR
         //#pragma endregion
         
         //#pragma region Logical operators
            value logical_not(); // TRUTH_NOT_EXPR
            value logical_and(value, bool short_circuit = true); // TRUTH_ANDIF_EXPR or TRUTH_AND_EXPR
            value logical_or(value, bool short_circuit = true); // TRUTH_ORIF_EXPR or TRUTH_OR_EXPR
            value logical_xor(value); // TRUTH_XOR_EXPR. never short-circuits
         //#pragma endregion
   };
   DECLARE_GCC_OPTIONAL_NODE_WRAPPER(value);
}

#include "gcc_wrappers/_node_boilerplate.undef.h"