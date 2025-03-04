#include "gcc_wrappers/value.h"
#include <cassert>
#include "gcc_headers/plugin-version.h"
#include <c-tree.h> // build_component_ref
#include <convert.h> // convert_to_integer and friends
#include <stringpool.h> // get_identifier
#include <c-family/c-common.h> // build_unary_op, lvalue_p

#include "gcc_wrappers/decl/base_value.h"
#include "gcc_wrappers/expr/base.h"
#include "gcc_wrappers/type/array.h"
#include "gcc_wrappers/type/pointer.h"

#include "gcc_wrappers/_node_boilerplate-impl.define.h"

namespace gcc_wrappers {
   GCC_NODE_WRAPPER_BOILERPLATE(value)
   
   /*static*/ bool value::raw_node_is(tree n) {
      if (decl::base_value::raw_node_is(n))
         return true;
      if (CONSTANT_CLASS_P(n) || EXPR_P(n))
         return true;
      return false;
   }
   
   decl::base_value value::as_decl() const {
      assert(decl::base_value::raw_node_is(this->_node));
      return decl::base_value::wrap(this->_node);
   }
   expr::base value::as_expr() const {
      assert(expr::base::raw_node_is(this->_node));
      return expr::base::wrap(this->_node);
   }
   
   type::base value::value_type() const {
      return type::base::wrap(TREE_TYPE(this->_node));
   }
   
   value value::access_member(const char* name) {
      assert(name != nullptr);
      auto vt = this->value_type();
      assert(vt.is_record() || vt.is_union());
      
      auto field_identifier = get_identifier(name);
      
      auto raw = build_component_ref(
         UNKNOWN_LOCATION, // source location: overall member-access operation
         this->_node,      // object to access on
         field_identifier, // member to access, as an IDENTIFIER_NODE
         UNKNOWN_LOCATION  // source location: field identifier
         #if GCCPLUGIN_VERSION_MAJOR >= 13 && GCCPLUGIN_VERSION_MINOR >= 1
            // GCC PR91134
            // https://github.com/gcc-mirror/gcc/commit/7a3ee77a2e33b8b8ad31aea27996ebe92a5c8d83
            ,
            UNKNOWN_LOCATION // source location: arrow operator, if present
            #if GCCPLUGIN_VERSION_MAJOR >= 14 && GCCPLUGIN_VERSION_MINOR >= 3
               // NOTE: As of this writing, this change hasn't actually shipped yet, and 
               //       the latest version of GCC is 14.2.0. Making a blind guess that 
               //       this will make it into 14.3.0.
               //
               // https://github.com/gcc-mirror/gcc/commit/bb49b6e4f55891d0d8b596845118f40df6ae72a5
               ,
               true // default; todo: use false for offsetof, etc.?
            #endif
         #endif
      );
      assert(raw != NULL_TREE);
      assert(TREE_CODE(raw) == COMPONENT_REF);
      return value::wrap(raw);
   }
   
   value value::access_array_element(value index) {
      assert(this->value_type().is_array());
      assert(index.value_type().is_integer());
      return value::wrap(build_array_ref(
         UNKNOWN_LOCATION,
         this->_node,
         index.unwrap()
      ));
   }
   
   // Creates a ARRAY_RANGE_REF.
   // This value must be of an ARRAY_TYPE.
   //value value::access_array_range(value start, value length); // TODO
   
   // Returns a value of a POINTER_TYPE.
   value value::address_of() {
      return value::wrap(build_unary_op(
         UNKNOWN_LOCATION,
         ADDR_EXPR,
         this->_node,
         false
      ));
   }
   
   value value::dereference() {
      auto vt = this->value_type();
      assert(vt.is_pointer()); // TODO: allow reference types too
      return value::wrap(build1(
         INDIRECT_REF,
         this->value_type().remove_pointer().unwrap(),
         this->_node
      ));
   }
   
   bool value::is_constant() const {
      return really_constant_p(this->_node);
   }
   bool value::is_lvalue() const {
      return lvalue_p(this->_node); // c-family/c-common.h, c/c-typeck.cc
   }
   
   bool value::is_associative_operator() const {
      return associative_tree_code(TREE_CODE(this->_node));
   }
   bool value::is_commutative_operator() const {
      return commutative_tree_code(TREE_CODE(this->_node));
   }
   
   //#pragma region Constant integer checks
      bool value::is_constant_int_zero() const {
         return integer_zerop(this->_node); // tree.h, tree.cc
      }
      bool value::is_constant_int_nonzero() const {
         return integer_nonzerop(this->_node); // tree.h, tree.cc
      }
      bool value::is_constant_int_one() const {
         return integer_onep(this->_node); // tree.h, tree.cc
      }
      bool value::is_constant_int_neg_one() const {
         return integer_minus_onep(this->_node); // tree.h, tree.cc
      }
      
      bool value::is_constant_true() const {
         return integer_truep(this->_node); // tree.h, tree.cc
      }
      
      bool value::is_constant_int_all_bits_set() const {
         return integer_all_onesp(this->_node); // tree.h, tree.cc
      }
      
      bool value::constant_has_single_bit() const {
         return integer_pow2p(this->_node); // tree.h, tree.cc
      }
      int value::constant_bit_floor() const {
         return tree_floor_log2(this->_node); // tree.h, tree.cc
      }
   //#pragma endregion
         
   //#pragma region Constant float/real checks
      bool value::is_constant_real_zero() const {
         return real_zerop(this->_node); // tree.h, tree.cc
      }
   //#pragma endregion
   
   //
   // Functions below create expressions, treating `this` as a unary 
   // operand or the lefthand operand (as appropriate). The functions 
   // assert that type requirements are met.
   //
   
   //#pragma region Conversions
      bool value::convertible_to_enum(type::enumeration desired) const {
         return this->convertible_to_integer(desired);
      }
      bool value::convertible_to_floating_point() const {
         // per `convert_to_real_1 `gcc/convert.cc`
         if (TREE_CODE(this->_node) == COMPOUND_EXPR) {
            return value::wrap(TREE_OPERAND(this->_node, 1)).convertible_to_floating_point();
         }
         switch (this->value_type().code()) {
            case REAL_TYPE:
               return true;
            case INTEGER_TYPE:
            case ENUMERAL_TYPE:
            case BOOLEAN_TYPE:
            #if IS_BITINT_TYPE_DEFINED
            case BITINT_TYPE:
            #endif
               return true;
            case FIXED_POINT_TYPE:
               return true;
            case COMPLEX_TYPE:
               return true;
            default:
               break;
         }
         return false;
      }
      bool value::convertible_to_integer(type::integral desired) const {
         // per `convert_to_integer_1 `gcc/convert.cc`
         if (desired.is_enum()) {
            if (!desired.is_complete())
               return false;
         }
         if (TREE_CODE(this->_node) == COMPOUND_EXPR) {
            return value::wrap(TREE_OPERAND(this->_node, 1)).convertible_to_integer(desired);
         }
         const auto vt = this->value_type();
         switch (vt.code()) {
            case POINTER_TYPE:
            case REFERENCE_TYPE:
               return true;
            case INTEGER_TYPE:
            case ENUMERAL_TYPE:
            case BOOLEAN_TYPE:
            case OFFSET_TYPE:
            #if IS_BITINT_TYPE_DEFINED
            case BITINT_TYPE:
            #endif
               return true;
            case REAL_TYPE:
               return true;
            case FIXED_POINT_TYPE:
               return true;
            case COMPLEX_TYPE:
               // comparing type sizes always tests as non-equal if neither is an integer constant
               if (!vt.is_complete() || !desired.is_complete()) {
                  return false;
               }
               if (vt.size_in_bits() != desired.size_in_bits()) {
                  return false;
               }
               return true;
            default:
               break;
         }
         return false;
      }
      bool value::convertible_to_pointer() const {
         // per `convert_to_pointer_1` in `gcc/convert.cc`
         switch (this->value_type().code()) {
            case POINTER_TYPE:
            case REFERENCE_TYPE:
               return true;
            case INTEGER_TYPE:
            case ENUMERAL_TYPE:
            case BOOLEAN_TYPE:
            #if IS_BITINT_TYPE_DEFINED
            case BITINT_TYPE:
            #endif
               return true;
            default:
               break;
         }
         return false;
      }
      bool value::convertible_to_vector(type::base desired) const {
         // per `convert_to_vector` in `gcc/convert.cc`
         const auto vt = this->value_type();
         switch (vt.code()) {
            case INTEGER_TYPE:
            case VECTOR_TYPE:
               // comparing type sizes always tests as non-equal if neither is an integer constant
               if (!vt.is_complete() || !desired.is_complete()) {
                  return false;
               }
               if (vt.size_in_bits() != desired.size_in_bits()) {
                  return false;
               }
               return true;
            default:
               break;
         }
         return false;
         
      }
      
      value value::convert_to_enum(type::enumeration desired) {
         assert(convertible_to_enum(desired));
         auto raw = ::convert_to_integer(desired.unwrap(), this->_node);
         assert(raw != error_mark_node); // if this fails, update the `convertible_to...` func
         return value::wrap(raw);
      }
      value value::convert_to_floating_point(type::floating_point desired) {
         assert(convertible_to_floating_point());
         auto raw = ::convert_to_real(desired.unwrap(), this->_node);
         assert(raw != error_mark_node); // if this fails, update the `convertible_to...` func
         return value::wrap(raw);
      }
      value value::convert_to_integer(type::integral desired) {
         assert(desired.is_integer());
         assert(convertible_to_integer(desired));
         auto raw = ::convert_to_integer(desired.unwrap(), this->_node); 
         assert(raw != error_mark_node); // if this fails, update the `convertible_to...` func
         return value::wrap(raw);
      }
      value value::convert_to_pointer(type::pointer desired) {
         assert(convertible_to_pointer());
         value out;
         auto raw = ::convert_to_pointer(desired.unwrap(), this->_node);
         assert(raw != error_mark_node); // if this fails, update the `convertible_to...` func
         return value::wrap(raw);
      }

      value value::convert_to_truth_value() {
         return value::wrap(c_common_truthvalue_conversion(UNKNOWN_LOCATION, this->_node));
      }
      
      value value::convert_array_to_pointer() {
         //
         // In mimicry of `array_to_pointer_conversion` in `c/c-typeck.cc`.
         //
         auto array_type = this->value_type().as_array();
         STRIP_TYPE_NOPS(this->_node);
         auto value_type = array_type.value_type();
         
         [[maybe_unused]] bool variably_modified = false;
         #ifdef C_TYPE_VARIABLY_MODIFIED
            variably_modified = C_TYPE_VARIABLY_MODIFIED(value_type.unwrap());
         #endif
         
         auto pointer_type = value_type.add_pointer();
         #ifdef C_TYPE_VARIABLY_MODIFIED
            C_TYPE_VARIABLY_MODIFIED(pointer_type.unwrap()) = variably_modified;
         #endif
         
         if (INDIRECT_REF_P(this->_node)) {
            return value::wrap(
               convert(
                  pointer_type.unwrap(),
                  TREE_OPERAND(this->_node, 0)
               )
            );
         }
         
         return value::wrap(
            convert(
               pointer_type.unwrap(),
               build_unary_op(
                  UNKNOWN_LOCATION,
                  ADDR_EXPR,
                  this->_node,
                  true // suppress conversion within `build_unary_op` since we do it with `convert`
               )
            )
         );
      }
      
      value value::conversion_sans_bytecode(type::base desired) {
         return value::wrap(build1(
            NOP_EXPR,
            desired.unwrap(),
            this->_node
         ));
      }
   
      value value::perform_default_promotions() {
         return value::wrap(default_conversion(this->_node));
      }
   //#pragma endregion
   
   value value::_make_cmp2(tree_code expr_type, value other) {
      //
      // TODO: permit pointer operands?
      //
      auto vt_t = this->value_type();
      auto vt_b = other.value_type();
      assert(vt_t.is_integer() || vt_t.is_floating_point());
      assert(vt_b.is_integer() || vt_b.is_floating_point());
      
      auto result_type = boolean_type_node;
      if (vt_t == vt_b && vt_t == type::base::wrap(integer_type_node)) {
         result_type = integer_type_node;
      }
      
      return value::wrap(build2(
         expr_type,
         result_type,
         this->_node,
         other._node
      ));
   }
   
   //#pragma region Comparison operators
      value value::cmp_is_less(value other) {
         return _make_cmp2(LT_EXPR, other);
      }
      value value::cmp_is_less_or_equal(value other) {
         return _make_cmp2(LE_EXPR, other);
      }
      value value::cmp_is_greater(value other) {
         return _make_cmp2(GT_EXPR, other);
      }
      value value::cmp_is_greater_or_equal(value other) {
         return _make_cmp2(GE_EXPR, other);
      }
      value value::cmp_is_equal(value other) {
         return _make_cmp2(EQ_EXPR, other);
      }
      value value::cmp_is_not_equal(value other) {
         return _make_cmp2(NE_EXPR, other);
      }
   //#pragma endregion
   
   value value::_make_arith1(tree_code expr_type) {
      auto vt = this->value_type();
      assert(vt.is_integer() || vt.is_floating_point());
      return value::wrap(build1(
         expr_type,
         this->value_type().unwrap(),
         this->_node
      ));
   }
   
   // Helper function for arith operations, requiring that both operands 
   // be arithmetic types and converting the second (`other`) operand to 
   // a type of the same TREE_CODE as the first's type, as needed.
   value value::_make_arith2(tree_code expr_type, value other) {
      auto vt_t = this->value_type();
      auto vt_b = other.value_type();
      assert(vt_t.is_integer() || vt_t.is_floating_point());
      assert(vt_b.is_integer() || vt_b.is_floating_point());
      
      value arg = other;
      if (vt_t.is_integer()) {
         if (!vt_b.is_integer())
            arg = arg.convert_to_integer(vt_t.as_integral());
      } else if (vt_t.is_floating_point()) {
         if (!vt_b.is_floating_point())
            arg = arg.convert_to_floating_point(vt_t.as_floating_point());
      }
      
      return value::wrap(build2(
         expr_type,
         vt_t.unwrap(),
         this->_node,
         arg._node
      ));
   }
   
   value value::_make_bitwise2(tree_code expr_type, value other) {
      auto vt = this->value_type();
      assert(vt.is_integer());
      assert(other.value_type().is_integer());
      
      return value::wrap(build2(
         expr_type,
         vt.unwrap(),
         this->_node,
         other._node
      ));
   }
   
   //#pragma region Arithmetic operators
      value value::arith_abs() {
         return _make_arith1(ABS_EXPR);
      }
      value value::arith_neg() {
         return _make_arith1(NEGATE_EXPR);
      }
            
      value value::complex_conjugate() {
         auto vt = value_type();
         assert(vt.is_integer() || vt.is_floating_point() || vt.is_complex());
         
         return value::wrap(build1(
            CONJ_EXPR,
            this->value_type().unwrap(),
            this->_node
         ));
      }
      
      value value::decrement_pre() {
         return value::wrap(build_unary_op(
            UNKNOWN_LOCATION,
            PREDECREMENT_EXPR,
            this->_node,
            false
         ));
      }
      value value::decrement_post() {
         return value::wrap(build_unary_op(
            UNKNOWN_LOCATION,
            POSTDECREMENT_EXPR,
            this->_node,
            false
         ));
      }
      value value::increment_pre() {
         return value::wrap(build_unary_op(
            UNKNOWN_LOCATION,
            PREINCREMENT_EXPR,
            this->_node,
            false
         ));
      }
      value value::increment_post() {
         return value::wrap(build_unary_op(
            UNKNOWN_LOCATION,
            POSTINCREMENT_EXPR,
            this->_node,
            false
         ));
      }
      
      value value::decrement_pre(value by) {
         return _make_arith2(PREDECREMENT_EXPR, by);
      }
      value value::decrement_post(value by) {
         return _make_arith2(POSTDECREMENT_EXPR, by);
      }
      value value::increment_pre(value by) {
         return _make_arith2(PREINCREMENT_EXPR, by);
      }
      value value::increment_post(value by) {
         return _make_arith2(POSTINCREMENT_EXPR, by);
      }
      
      value value::add(value other) {
         return _make_arith2(PLUS_EXPR, other);
      }
      value value::sub(value other) {
         return _make_arith2(MINUS_EXPR, other);
      }
      value value::mul(value other) {
         return _make_arith2(MULT_EXPR, other);
      }
      value value::mod(value other) {
         auto vt = this->value_type();
         assert(vt.is_integer());
         assert(other.value_type().is_integer());
         
         return value::wrap(build2(
            TRUNC_MOD_EXPR,
            vt.unwrap(),
            this->_node,
            other._node
         ));
      }
      
      value value::div(value other, round_value_toward rounding) {
         auto vt_a = this->value_type();
         auto vt_b = other.value_type();
         auto arg  = other;
         
         assert(vt_a.is_integer() || vt_a.is_floating_point());
         assert(vt_b.is_integer() || vt_b.is_floating_point());
         
         if (vt_a.is_floating_point()) {
            if (!vt_b.is_floating_point())
               arg = arg.convert_to_floating_point(vt_a.as_floating_point());
            
            return value::wrap(build2(
               RDIV_EXPR,
               vt_a.unwrap(),
               this->_node,
               arg._node
            ));
         } else {
            if (!vt_b.is_integer())
               arg = arg.convert_to_integer(vt_a.as_integral());
            
            tree_code code;
            switch (rounding) {
               case round_value_toward::negative_infinity:
                  code = FLOOR_DIV_EXPR;
                  break;
               case round_value_toward::zero:
               default:
                  code = TRUNC_DIV_EXPR;
                  break;
               case round_value_toward::nearest_integer:
                  code = ROUND_DIV_EXPR;
                  break;
               case round_value_toward::positive_infinity:
                  code = CEIL_DIV_EXPR;
                  break;
            }
            
            return value::wrap(build2(
               code,
               vt_a.unwrap(),
               this->_node,
               arg._node
            ));
         }
      }
      
      value value::min(value other) {
         return value::wrap(build_binary_op(
            UNKNOWN_LOCATION,
            MIN_EXPR,
            this->_node,
            other._node,
            false
         ));
      }
      value value::max(value other) {
         return value::wrap(build_binary_op(
            UNKNOWN_LOCATION,
            MAX_EXPR,
            this->_node,
            other._node,
            false
         ));
      }
   //#pragma endregion
   
   //#pragma region Bitwise operators, available only on integers
      value value::bitwise_and(value other) {
         return _make_bitwise2(BIT_AND_EXPR, other);
      }
      value value::bitwise_not() {
         auto vt = this->value_type();
         assert(vt.is_integer());
         return value::wrap(build1(
            BIT_NOT_EXPR,
            vt.unwrap(),
            this->_node
         ));
      }
      value value::bitwise_or(value other) {
         return _make_bitwise2(BIT_IOR_EXPR, other);
      }
      value value::bitwise_xor(value other) {
         return _make_bitwise2(BIT_XOR_EXPR, other);
      }
      value value::shift_left(value bitcount) {
         return _make_bitwise2(LSHIFT_EXPR, bitcount);
      }
      value value::shift_right(value bitcount) {
         return _make_bitwise2(RSHIFT_EXPR, bitcount);
      }
   //#pragma endregion
   
   //#pragma region Logical operators
      value value::logical_not() {
         return value::wrap(build1(
            TRUTH_NOT_EXPR,
            truthvalue_type_node,
            c_common_truthvalue_conversion(UNKNOWN_LOCATION, this->_node)
         ));
      }
      value value::logical_and(value other, bool short_circuit) {
         const auto code = short_circuit ? TRUTH_ANDIF_EXPR : TRUTH_AND_EXPR;
      
         return value::wrap(build2(
            code,
            truthvalue_type_node,
            c_common_truthvalue_conversion(UNKNOWN_LOCATION, this->_node),
            c_common_truthvalue_conversion(UNKNOWN_LOCATION, other.unwrap())
         ));
      }
      value value::logical_or(value other, bool short_circuit) {
         const auto code = short_circuit ? TRUTH_ORIF_EXPR : TRUTH_OR_EXPR;
      
         return value::wrap(build2(
            code,
            truthvalue_type_node,
            c_common_truthvalue_conversion(UNKNOWN_LOCATION, this->_node),
            c_common_truthvalue_conversion(UNKNOWN_LOCATION, other.unwrap())
         ));
      }
      value value::logical_xor(value other) {
         return value::wrap(build2(
            TRUTH_XOR_EXPR,
            truthvalue_type_node,
            c_common_truthvalue_conversion(UNKNOWN_LOCATION, this->_node),
            c_common_truthvalue_conversion(UNKNOWN_LOCATION, other.unwrap())
         ));
      }
   //#pragma endregion
}