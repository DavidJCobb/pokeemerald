#include "bitpacking/transform_function_validation_helpers.h"
#include "lu/stringf.h"
#include "gcc_wrappers/type/function.h"
namespace gw {
   using namespace gcc_wrappers;
}

#include <diagnostic.h>

namespace bitpacking {
   extern bool can_be_transform_function(gw::decl::function decl) {
      auto type = decl.function_type();
      auto args = type.arguments();
      if (!args)
         return false;
      if (args->size() != 3)
         return false;
      if (type.is_varargs())
         return false;
      
      auto arg_a = type.nth_argument_type(0);
      auto arg_b = type.nth_argument_type(1);
      if (!arg_a.is_pointer() || !arg_b.is_pointer())
         return false;
      
      return true;
   }
   extern bool can_be_pre_pack_function(gw::decl::function decl) {
      if (!can_be_transform_function(decl))
         return false;
      auto type = decl.function_type();
      if (type.nth_argument_type(1).remove_pointer().is_const())
         return false;
      return true;
   }
   extern bool can_be_post_unpack_function(gw::decl::function decl) {
      if (!can_be_transform_function(decl))
         return false;
      auto type = decl.function_type();
      if (type.nth_argument_type(0).remove_pointer().is_const())
         return false;
      return true;
   }
   
   extern std::string check_transform_functions_match_types(
      gw::decl::function pre_pack,
      gw::decl::function post_unpack
   ) {
      auto type_a = pre_pack.function_type();
      auto type_b = post_unpack.function_type();
      
      auto normal_type_a = type_a.nth_argument_type(0).remove_pointer().remove_const();
      auto normal_type_b = type_b.nth_argument_type(0).remove_pointer().remove_const();
      if (normal_type_a != normal_type_b) {
         auto as = normal_type_a.pretty_print();
         auto bs = normal_type_b.pretty_print();
         return lu::stringf("the normal-type arguments don't match (%<%s%> for pre-pack versus %<%s%> for post-unpack)", as.c_str(), bs.c_str());
      }
      
      auto packed_type_a = type_a.nth_argument_type(1).remove_pointer().remove_const();
      auto packed_type_b = type_b.nth_argument_type(1).remove_pointer().remove_const();
      if (packed_type_a != packed_type_b) {
         auto as = packed_type_a.pretty_print();
         auto bs = packed_type_b.pretty_print();
         return lu::stringf("the packed-type arguments don't match (%<%s%> for pre-pack versus %<%s%> for post-unpack)", as.c_str(), bs.c_str());
      }
      return {};
   }
   
   extern gw::type::optional_base get_in_situ_type(
      gw::decl::function pre_pack,
      gw::decl::function post_unpack
   ) {
      auto type_a = pre_pack.function_type();
      auto type_b = post_unpack.function_type();
      
      auto normal_type_a = type_a.nth_argument_type(0).remove_pointer().remove_const();
      auto normal_type_b = type_b.nth_argument_type(0).remove_pointer().remove_const();
      if (normal_type_a != normal_type_b)
         return {};
      return normal_type_a;
   }
   
   extern gw::type::optional_base get_transformed_type(
      gw::decl::function pre_pack,
      gw::decl::function post_unpack
   ) {
      auto type_a = pre_pack.function_type();
      auto type_b = post_unpack.function_type();
      
      auto packed_type_a = type_a.nth_argument_type(1).remove_pointer().remove_const();
      auto packed_type_b = type_b.nth_argument_type(1).remove_pointer().remove_const();
      if (packed_type_a != packed_type_b)
         return {};
      return packed_type_a;
   }
}