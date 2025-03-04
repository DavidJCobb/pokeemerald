#include "gcc_wrappers/type/base.h"
#include "gcc_wrappers/decl/type_def.h"
#include "gcc_wrappers/identifier.h"
#include <cassert>
#include <charconv> // std::to_string, for pretty-printing array extents
#include <c-family/c-common.h> // c_build_qualified_type, c_type_promotes_to

// GCC flattens most directories when it installs plug-in headers. In GCC source 
// code, this is `c/c-tree.h` but for plug-ins, it's just `c-tree.h`.
#include <c-tree.h> // comptypes

#include "gcc_headers/plugin-version.h"

// For casts:
#include "gcc_wrappers/type/array.h"
#include "gcc_wrappers/type/container.h"
#include "gcc_wrappers/type/enumeration.h"
#include "gcc_wrappers/type/fixed_point.h"
#include "gcc_wrappers/type/floating_point.h"
#include "gcc_wrappers/type/function.h"
#include "gcc_wrappers/type/integral.h"
#include "gcc_wrappers/type/pointer.h"
#include "gcc_wrappers/type/record.h"
#include "gcc_wrappers/type/untagged_union.h"

#include "gcc_wrappers/_node_boilerplate-impl.define.h"

namespace gcc_wrappers::type {
   GCC_NODE_WRAPPER_BOILERPLATE(base)
   
   bool base::operator==(const base other) const {
      if (this->_node == other._node)
         return true;
      
      gcc_assert(c_language == c_language_kind::clk_c);
      return comptypes(this->_node, other._node) != 0;
   }
   
   std::string base::name() const {
      optional_node name = TYPE_NAME(this->_node);
      if (!name)
         return {}; // unnamed struct
      if (name->is<identifier>()) {
         return std::string(name->as<identifier>().name());
      }
      if (name->is<decl::type_def>()) {
         //
         // `this` is a synonym type created by the `typedef` keyword.
         //
         return std::string(name->as<decl::type_def>().name());
      }
      //
      // Name irretrievable?
      //
      return {};
   }
   std::string base::tag_name() const {
      if (!this->is_enum() && !this->is_record() && !this->is_union())
         return {};
      optional_node name = TYPE_NAME(this->_node);
      if (!name || !name->is<identifier>())
         return {};
      return std::string(name->as<identifier>().name());
   }
   std::string base::pretty_print() const {
      std::string out;
      
      //
      // Try printing function types and function pointer types.
      //
      {
         bool is_const_pointer    = false;
         bool is_function_pointer = false;
         
         optional_function ft;
         
         if (is_function()) {
            ft = this->as_function();
         } else if (is_pointer()) {
            auto pt = remove_pointer();
            if (pt.is_function()) {
               ft = pt.as_function();
               is_function_pointer = true;
               is_const_pointer    = ft->is_const();
            }
         }
         if (ft) {
            out = ft->return_type().pretty_print();
            if (is_function_pointer) {
               out += "(*";
               if (is_const_pointer) {
                  out += " const";
               }
               out += ')';
            } else {
               out += "(&)"; // not sure if this is correct
            }
            out += '(';
            if (!ft->is_unprototyped()) {
               bool first   = true;
               bool varargs = true;
               auto args    = ft->arguments();
               if (args) {
                  args->for_each_value([&first, &out, &varargs](optional_node arg) {
                     assert(arg);
                     if (arg.unwrap() == void_type_node) {
                        varargs = false;
                        return;
                     }
                     base t = (*arg).as<base>();
                     if (first) {
                        first = false;
                     } else {
                        out += ", ";
                     }
                     out += t.pretty_print();
                  });
               }
               if (varargs) {
                  if (!first)
                     out += ", ";
                  out += "...";
               }
            }
            out += ')';
            return out;
         }
      }
      
      //
      // Try printing array types.
      //
      if (is_array()) {
         auto at = this->as_array();
         out = at.value_type().pretty_print();
         if (is_restrict()) { // array types can be `restrict` in C23
            out += " restrict";
         }
         out += '[';
         if (auto extent = at.extent(); extent.has_value()) {
            char buffer[64] = { 0 };
            auto result = std::to_chars(
               (char*)buffer,
               (char*)(&buffer + sizeof(buffer) - 1),
               *extent
            );
            if (result.ec == std::errc{}) {
               *result.ptr = '\0';
            } else {
               buffer[0] = '?';
               buffer[1] = '\0';
            }
            out += buffer;
         }
         out += ']';
         return out;
      }
      
      //
      // Try printing pointer types.
      //
      if (is_pointer()) {
         out += remove_pointer().pretty_print();
         out += '*';
         if (is_const())
            out += " const";
         if (is_restrict())
            out += " restrict";
         return out;
      }
      
      //
      // Base case.
      //
      
      if (is_const())
         out = "const ";
      
      auto id = TYPE_NAME(this->_node);
      if (id == NULL_TREE) {
         out += "<unnamed>";
      } else {
         switch (TREE_CODE(id)) {
            case IDENTIFIER_NODE:
               out += IDENTIFIER_POINTER(id);
               break;
            case TYPE_DECL:
               out += IDENTIFIER_POINTER(DECL_NAME(id));
               break;
            default:
               out += "<unnamed>";
               break;
         }
      }
      
      return out;
   }
   
   location_t base::source_location() const {
      auto decl = this->declaration();
      if (!decl)
         return UNKNOWN_LOCATION;
      return decl->source_location();
   }
   
   attribute_list base::attributes() {
      return attribute_list::wrap(TYPE_ATTRIBUTES(this->_node));
   }
   const attribute_list base::attributes() const {
      return attribute_list::wrap(TYPE_ATTRIBUTES(this->_node));
   }
   
   base base::canonical() const {
      return base::wrap(TYPE_CANONICAL(this->_node));
   }
   base base::main_variant() const {
      return base::wrap(TYPE_MAIN_VARIANT(this->_node));
   }
   
   decl::optional_type_def base::declaration() const {
      optional_node name = TYPE_NAME(this->_node);
      if (name && name->is<decl::type_def>())
         return name->as<decl::type_def>();
      return {};
   }
   
   bool base::is_type_or_transitive_typedef_thereof(base other) const {
      if (this->is_same(other))
         return true;
      
      optional_base current = this->_node;
      do {
         auto decl = current->declaration();
         if (!decl)
            return false;
         auto synonym = decl->is_synonym_of();
         if (!synonym)
            return false;
         if (synonym->is_same(other))
            return true;
         current = synonym;
      } while (current);
      
      return false;
   }
   
   bool base::has_user_requested_alignment() const {
      return TYPE_USER_ALIGN(this->_node);
   }
   bool base::is_complete() const {
      return COMPLETE_TYPE_P(this->_node);
   }
   bool base::is_packed() const {
      return TYPE_PACKED(this->_node);
   }
   
   size_t base::alignment_in_bits() const {
      return TYPE_ALIGN(this->_node);
   }
   size_t base::alignment_in_bytes() const {
      return TYPE_ALIGN_UNIT(this->_node);
   }
   size_t base::minimum_alignment_in_bits() const {
      return TYPE_WARN_IF_NOT_ALIGN(this->_node);
   }
   size_t base::size_in_bits() const {
      auto size_node = TYPE_SIZE(this->_node);
      if (TREE_CODE(size_node) != INTEGER_CST)
         return 0;
      
      // This truncates, but if you have a type that's larger than 
      // (std::numeric_limits<uint32_t>::max() / 8), then you're 
      // probably screwed anyway tbh
      return TREE_INT_CST_LOW(size_node);
   }
   size_t base::size_in_bytes() const {
      return this->size_in_bits() / 8;
   }
   
   bool base::is_arithmetic() const {
      switch (this->code()) {
         case BOOLEAN_TYPE:
         case COMPLEX_TYPE:
         case ENUMERAL_TYPE:
         case FIXED_POINT_TYPE:
         case INTEGER_TYPE:
         case REAL_TYPE:
            return true;
         default:
            break;
      }
      if (this->is_bitint())
         return true;
      return false;
      
      auto code = TREE_CODE(this->_node);
      return code == INTEGER_TYPE || code == REAL_TYPE;
   }
   bool base::is_array() const {
      return TREE_CODE(this->_node) == ARRAY_TYPE;
   }
   bool base::is_bitint() const {
      #if GCCPLUGIN_VERSION_MAJOR >= 14
         return TREE_CODE(this->_node) == BITINT_TYPE;
      #else
         return false;
      #endif
   }
   bool base::is_boolean() const {
      return TREE_CODE(this->_node) == BOOLEAN_TYPE;
   }
   bool base::is_complex() const {
      return TREE_CODE(this->_node) == COMPLEX_TYPE;
   }
   bool base::is_container() const {
      return container::raw_node_is(this->_node);
   }
   bool base::is_enum() const {
      return TREE_CODE(this->_node) == ENUMERAL_TYPE;
   }
   bool base::is_fixed_point() const {
      return TREE_CODE(this->_node) == FIXED_POINT_TYPE;
   }
   bool base::is_floating_point() const {
      return TREE_CODE(this->_node) == REAL_TYPE;
   }
   bool base::is_function() const {
      auto code = TREE_CODE(this->_node);
      return code == FUNCTION_TYPE || code == METHOD_TYPE;
   }
   bool base::is_integer() const {
      return TREE_CODE(this->_node) == INTEGER_TYPE;
   }
   bool base::is_method() const {
      return TREE_CODE(this->_node) == METHOD_TYPE;
   }
   bool base::is_null_pointer_type() const {
      return TREE_CODE(this->_node) == NULLPTR_TYPE;
   }
   bool base::is_record() const {
      return TREE_CODE(this->_node) == RECORD_TYPE;
   }
   bool base::is_union() const {
      return TREE_CODE(this->_node) == UNION_TYPE;
   }
   bool base::is_vector() const {
      return TREE_CODE(this->_node) == VECTOR_TYPE;
   }
   bool base::is_void() const {
      return TREE_CODE(this->_node) == VOID_TYPE;
   }
   
   bool base::has_c_boolean_semantics() const {
      if (this->is_boolean())
         return true;
      #if GCCPLUGIN_VERSION_MAJOR >= 13 && GCCPLUGIN_VERSION_MINOR >= 1
         if (!this->is_enum())
            return false;
         
         auto underlying = ENUM_UNDERLYING_TYPE(this->_node);
         if (underlying == NULL_TREE)
            return false;
         return TREE_CODE(underlying) == BOOLEAN_TYPE;
      #else
         return false;
      #endif
   }
   
   //
   // Casts:
   //
   
   array base::as_array() {
      return array::wrap(this->_node);
   }
   const array base::as_array() const {
      return array::wrap(this->_node);
   }
   
   container base::as_container() {
      return container::wrap(this->_node);
   }
   const container base::as_container() const {
      return container::wrap(this->_node);
   }
   
   enumeration base::as_enum() {
      return enumeration::wrap(this->_node);
   }
   const enumeration base::as_enum() const {
      return enumeration::wrap(this->_node);
   }
   
   floating_point base::as_floating_point() {
      return floating_point::wrap(this->_node);
   }
   const floating_point base::as_floating_point() const {
      return floating_point::wrap(this->_node);
   }
   
   fixed_point base::as_fixed_point() {
      return fixed_point::wrap(this->_node);
   }
   const fixed_point base::as_fixed_point() const {
      return fixed_point::wrap(this->_node);
   }
         
   function base::as_function() {
      return function::wrap(this->_node);
   }
   const function base::as_function() const {
      return function::wrap(this->_node);
   }
   
   integral base::as_integral() {
      return integral::wrap(this->_node);
   }
   const integral base::as_integral() const {
      return integral::wrap(this->_node);
   }
   
   pointer base::as_pointer() {
      return pointer::wrap(this->_node);
   }
   const pointer base::as_pointer() const {
      return pointer::wrap(this->_node);
   }
   
   record base::as_record() {
      return record::wrap(this->_node);
   }
   const record base::as_record() const {
      return record::wrap(this->_node);
   }
   
   untagged_union base::as_union() {
      return untagged_union::wrap(this->_node);
   }
   const untagged_union base::as_union() const {
      return untagged_union::wrap(this->_node);
   }
   
   //
   // Queries and transformations:
   //
   
   base base::_with_qualifiers_added(int change) const {
      auto qual = TYPE_QUALS(this->_node);
      if ((qual & change) == change)
         return *this;
      qual |= change;
      return base::wrap(c_build_qualified_type(this->_node, qual));
   }
   base base::_with_qualifiers_removed(int change) const {
      auto qual = TYPE_QUALS(this->_node);
      if (!(qual & change))
         return *this;
      qual &= ~change;
      return base::wrap(c_build_qualified_type(this->_node, qual));
   }
   base base::_with_qualifiers_replaced(int qual) const {
      return base::wrap(c_build_qualified_type(this->_node, qual));
   }
   
   base base::with_all_qualifiers_stripped() const {
      return base::wrap(c_build_qualified_type(this->_node, 0));
   }
   
   bool base::is_atomic() const {
      return TYPE_ATOMIC(this->_node);
   }
   base base::add_atomic() const {
      return _with_qualifiers_added(TYPE_QUAL_ATOMIC);
   }
   base base::remove_atomic() const {
      return _with_qualifiers_removed(TYPE_QUAL_ATOMIC);
   }
   
   base base::add_const() const {
      return _with_qualifiers_added(TYPE_QUAL_CONST);
   }
   base base::remove_const() const {
      return _with_qualifiers_removed(TYPE_QUAL_CONST);
   }
   bool base::is_const() const {
      return TREE_READONLY(this->_node);
   }
   
   bool base::is_volatile() const {
      return TYPE_VOLATILE(this->_node);
   }
   base base::add_volatile() const {
      return _with_qualifiers_added(TYPE_QUAL_VOLATILE);
   }
   base base::remove_volatile() const {
      return _with_qualifiers_removed(TYPE_QUAL_VOLATILE);
   }
   
   pointer base::add_pointer() const {
      return pointer::wrap(build_pointer_type(this->_node));
   }
   base base::remove_pointer() const {
      if (!is_pointer())
         return *this;
      return base::wrap(TREE_TYPE(this->_node));
   }
   bool base::is_pointer() const {
      return TREE_CODE(this->_node) == POINTER_TYPE;
   }
   
   bool base::is_reference() const {
      return TREE_CODE(this->_node) == REFERENCE_TYPE;
   }
   bool base::is_lvalue_reference() const {
      return this->is_reference() && !TYPE_REF_IS_RVALUE(this->_node);
   }
   bool base::is_rvalue_reference() const {
      return this->is_reference() && TYPE_REF_IS_RVALUE(this->_node);
   }
   
   bool base::is_restrict() const {
      return TYPE_RESTRICT(this->_node);
   }
   bool base::is_restrict_allowed() const {
      // See source for `c_build_qualified_type`
      if (!POINTER_TYPE_P(this->_node))
         return false;
      if (!C_TYPE_OBJECT_OR_INCOMPLETE_P(this->_node))
         return false;
      return true;
   }
   base base::add_restrict() const {
      gcc_assert(is_restrict_allowed());
      return _with_qualifiers_added(TYPE_QUAL_RESTRICT);
   }
   base base::remove_restrict() const {
      return _with_qualifiers_removed(TYPE_QUAL_RESTRICT);
   }
   
   base base::with_user_defined_alignment(size_t bytes) {
      return with_user_defined_alignment_in_bits(bytes * 8);
   }
   base base::with_user_defined_alignment_in_bits(size_t bits) {
      assert(!is_packed());
      return base::wrap(build_aligned_type(this->_node, bits));
   }
   
   //
   // Tests:
   //
   
   bool base::is_assignable_to(base lhs, const assign_check_options& options) const {
      const auto rhs     = *this;
      const_tree lhs_raw = lhs.unwrap();
      const_tree rhs_raw = rhs.unwrap();
      
      //
      // Logic here is based on GCC's `convert_for_assignment` in 
      // `c/c-typeck.cc`.
      //
      
      // Identity and equality comparisons.
      const bool same_main  = lhs.main_variant().is_same(rhs.main_variant());
      const bool equal_main = lhs.main_variant().is_equal(rhs.main_variant());
      
      if (same_main)
         return true;
      
      //
      // Check for implicit conversions below. (There are cases where 
      // it'd be enough for `equal_main`, but if those aren't checked 
      // for below, it's because they're a subset of allowed implicit 
      // conversions.)
      //
      
      if (options.require_cpp_compat) {
         //
         // In C, enums are implicitly interconvertible, and enums and 
         // integrals are also interconvertible. This is not the case 
         // in C++.
         //
         if (lhs.is_enum()) {
            return false;
         }
      }
      
      if (rhs.is_void())
         return false;
      if (!rhs.is_complete())
         return false;
      
      /*//
      if (lhs.is_reference() && !rhs.is_reference()) {
         // Non-references can be assigned to references. Wrap RHS in a 
         // reference and then test that against LHS.
         //
         // Not implemented here because references are C++ only and we're 
         // currently making a C plug-in; don't have the opportunity to test 
         // the relevant functionality.
      }
      //*/
      
      if (lhs.is_vector() && rhs.is_vector()) {
         return vector_types_convertible_p(lhs_raw, rhs_raw, false);
      }
      if (lhs.is_arithmetic() && rhs.is_arithmetic()) {
         // Arithmetic types can all interconvert, with enum being treated like 
         // int.
         return true;
      }
      
      if (lhs.is_record() || lhs.is_union()) {
         if (lhs.code() == rhs.code()) {
            // Aggregate types defined in different translation units.
            if (equal_main)
               return true;
         }
         
         if (options.as_function_argument) {
            //
            // Allow conversion to a transparent struct or union from its member 
            // type. That is: the following function accepts an argument of type 
            // `float`:
            //
            //    struct transparent {
            //       float foo;
            //    };
            //
            //    void func(transparent);
            //
            if (TYPE_TRANSPARENT_AGGR(lhs_raw)) {
               bool any_legal  = false;
               bool is_optimal = false;
               lhs.as_container().for_each_field([lhs, rhs, &any_legal, &is_optimal](decl::field decl) -> bool {
                  auto decl_type = decl.value_type();
                  if (decl_type.main_variant() == rhs.main_variant()) {
                     any_legal = is_optimal = true;
                     return false;
                  }
                  if (rhs.is_pointer() && decl_type.is_pointer()) {
                     auto deref_lhs = decl_type.remove_pointer();
                     auto deref_rhs = rhs.remove_pointer();
                     if (
                        (deref_lhs.is_void() && !deref_lhs.is_atomic()) ||
                        (deref_rhs.is_void() && !deref_rhs.is_atomic()) ||
                        decl_type.as_pointer().same_target_and_address_space_as(rhs.as_pointer())
                     ) {
                        any_legal = true;
                        
                        auto quals_lhs = TYPE_QUALS(deref_lhs.unwrap());
                        auto quals_rhs = TYPE_QUALS(deref_rhs.unwrap());
                        quals_lhs &= ~TYPE_QUAL_ATOMIC;
                        quals_rhs &= ~TYPE_QUAL_ATOMIC;
                        if (quals_lhs == quals_rhs) {
                           is_optimal = true;
                           return false;
                        }
                        if (deref_lhs.is_function() && deref_rhs.is_function()) {
                           if ((quals_lhs | quals_rhs) == quals_rhs) {
                              is_optimal = true;
                              return false;
                           }
                        } else {
                           if ((quals_lhs | quals_rhs) == quals_lhs) {
                              is_optimal = true;
                              return false;
                           }
                        }
                        return true;
                     }
                  }
                  return true;
               });
               return any_legal;
            }
         }
      }
      
      // Handle implicit conversions between pointer types.
      if (lhs.is_pointer() && rhs.is_pointer()) {
         // NOTE: Skipping handling of pointers to built-in functions.
         auto lhs_p = lhs.as_pointer();
         auto rhs_p = rhs.as_pointer();
         if (!options.rhs_is_null_pointer_constant && !lhs_p.compatible_address_space_with(rhs_p))
            return false;
         
         auto deref_lhs = lhs.remove_pointer();
         auto deref_rhs = rhs.remove_pointer();
         auto main_lhs  = deref_lhs.main_variant();
         auto main_rhs  = deref_rhs.main_variant();
         if (deref_lhs.is_atomic())
            main_lhs = main_lhs._with_qualifiers_replaced(TYPE_QUAL_ATOMIC);
         if (deref_rhs.is_atomic())
            main_rhs = main_rhs._with_qualifiers_replaced(TYPE_QUAL_ATOMIC);
         
         bool is_opaque_pointer = vector_targets_convertible_p(deref_lhs.unwrap(), deref_rhs.unwrap());
         
         /*//
         if (
            flag_plan9_extensions &&
            (main_lhs.is_record() || main_lhs.is_union()) &&
            (main_rhs.is_record() || main_rhs.is_union()) &&
            main_lhs != main_rhs
         ) {
            // The Plan 9 compiler allows a pointer-to-struct to be implicitly 
            // converted to a pointer to an anonymous field within the struct.
         }
         //*/
         
         if (options.require_cpp_compat) {
            // Forbid implicit conversions from void pointers to non-void pointers.
            if (deref_rhs.is_void() && !deref_lhs.is_void())
               return false;
         }
         
         // Pointers to non-functions can convert to a possibly-cv-qualified 
         // void pointer, and vice versa, but the pointed-to LHS type must have 
         // all of the qualifiers of the pointed-to RHS type.
         if (
            (deref_lhs.is_void() && !deref_lhs.is_atomic()) ||
            (deref_rhs.is_void() && !deref_rhs.is_atomic()) ||
            lhs_p.same_target_and_address_space_as(rhs_p)   ||
            is_opaque_pointer ||
            (
               c_common_unsigned_type(main_lhs.unwrap()) ==
               c_common_unsigned_type(main_rhs.unwrap())
               &&
               c_common_signed_type(main_lhs.unwrap()) ==
               c_common_signed_type(main_rhs.unwrap())
               &&
               main_lhs.is_atomic() == main_rhs.is_atomic()
            )
         ) {
            if (
               (deref_lhs.is_void() && deref_rhs.is_function()) ||
               (deref_lhs.is_function() && deref_rhs.is_void() && !options.rhs_is_null_pointer_constant)
            ) {
               //
               // You can't assign a function pointer to void*, nor a non-null 
               // void pointer to a function pointer.
               //
               return false;
            }
            return true;
         }
         return false;
      }
      //
      // Handle all other assignments to pointers.
      //
      if (lhs.is_null_pointer_type() && options.rhs_is_null_pointer_constant) {
         return true;
      }
      if (lhs.is_pointer()) {
         if (options.rhs_is_null_pointer_constant) {
            if (rhs.is_integer() || rhs.is_null_pointer_type() || rhs.is_bitint()) {
               return true;
            }
         }
         return false;
      }
      
      // Allow pointer-to-boolean conversions.
      if (lhs.has_c_boolean_semantics() && (rhs.is_pointer() || rhs.is_null_pointer_type())) {
         return true;
      }
      
      return false;
   }
}