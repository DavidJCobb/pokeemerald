#include "codegen/instructions/single.h"
#include <cassert>
#include "attribute_handlers/helpers/type_transitively_has_attribute.h"
#include "codegen/instructions/utils/generation_context.h"
#include "gcc_wrappers/constant/floating_point.h"
#include "gcc_wrappers/constant/integer.h"
#include "gcc_wrappers/constant/string.h"
#include "gcc_wrappers/decl/function.h"
#include "gcc_wrappers/expr/assign.h"
#include "gcc_wrappers/expr/call.h"
#include "gcc_wrappers/expr/local_block.h"
#include "gcc_wrappers/type/array.h"
#include "gcc_wrappers/type/base.h"
#include "gcc_wrappers/attribute.h"
#include "gcc_wrappers/attribute_list.h"
#include "gcc_wrappers/builtin_types.h"
#include "gcc_wrappers/value.h"
#include "bitpacking/global_options.h"
#include "basic_global_state.h"
namespace gw {
   using namespace gcc_wrappers;
}
namespace typed_options {
   using namespace bitpacking::typed_data_options::computed;
}

namespace codegen::instructions {
   bool single::is_omitted_and_defaulted() const {
      bool omitted   = false;
      bool defaulted = false;
      for(auto& segm : this->value.segments) {
         auto  pair = segm.descriptor();
         auto* desc = pair.read;
         if (desc == nullptr)
            continue;
         if (desc->options.is_omitted)
            omitted = true;
         if (desc->is_or_contains_defaulted())
            defaulted = true;
         
         if (omitted && defaulted)
            return true;
      }
      return omitted && defaulted;
   }
   
   static gw::expr::base _default_a_string(
      const value_path&   value_path,
      optional_value_pair value
   ) {
      auto& bgs = basic_global_state::get();
      assert(!!bgs.builtin_functions.memcpy);
      assert(!!bgs.builtin_functions.memset);
      
      const auto& ty      = gw::builtin_types::get();
      const auto& options = value_path.bitpacking_options();
      
      // Must be called for a string-type default value.
      assert(!!options.default_value);
      // TODO: constness ends up being pretty ugly in the GCC wrappers; can we improve it?
      auto defval = gw::node::wrap((tree) options.default_value.unwrap());
      assert(defval.is<gw::constant::string>());
      
      // Must be called for a to-be-defaulted string.
      assert(value.read->value_type().is_array());
      
      gw::type::array array_type = value.read->value_type().as_array();
      gw::type::base  char_type  = array_type.value_type();
      
      auto      str     = defval.as<gw::constant::string>();
      gw::value literal = str.to_string_literal(char_type);
      
      size_t char_size   = char_type.size_in_bytes();
      size_t string_size = str.size();
      size_t array_size  = *array_type.extent();
      
      if (options.has_attr_nonstring) {
         string_size -= 1; // exclude null terminator
      }
      
      auto call_memcpy = gw::expr::call(
         *bgs.builtin_functions.memcpy,
         // args:
         value.read->convert_array_to_pointer(),
         literal,
         gw::constant::integer(ty.size, string_size * char_size)
      );
      if (array_size <= string_size) {
         return call_memcpy;
      }
      auto call_memset = gw::expr::call(
         *bgs.builtin_functions.memset,
         // args:
         value.read->access_array_element(
            gw::constant::integer(ty.basic_int, string_size)
         ).address_of(),
         gw::constant::integer(ty.basic_int, 0),
         gw::constant::integer(ty.size, (array_size - string_size) * char_size)
      );
      
      gw::expr::local_block block;
      block.statements().append(call_memcpy);
      block.statements().append(call_memset);
      return block;
   }
   
   /*virtual*/ expr_pair single::generate(const utils::generation_context& ctxt) const {
      const auto& ty = gw::builtin_types::get();
      
      auto        value   = this->value.as_value_pair();
      const auto& options = this->value.bitpacking_options();
      const auto& global  = basic_global_state::get().global_options;
      
      //
      // Handle values that are omitted and defaulted.
      //
      if (options.is_omitted) {
         if (!options.default_value) {
            return expr_pair(
               gw::expr::base::wrap(build_empty_stmt(UNKNOWN_LOCATION)),
               gw::expr::base::wrap(build_empty_stmt(UNKNOWN_LOCATION))
            );
         }
         // TODO: constness ends up being pretty ugly in the GCC wrappers; can we improve it?
         auto defval = gw::node::wrap((tree) options.default_value.unwrap());
         //
         // Structs can't be defaulted, and this class should never be used 
         // to denote serialization of an array (unless it's a defaulted 
         // string), so we can just assume that we're operating on a string 
         // or a scalar.
         //
         if (defval.is<gw::constant::string>()) {
            return expr_pair(
               _default_a_string(this->value, value),
               gw::expr::base::wrap(build_empty_stmt(UNKNOWN_LOCATION))
            );
         }
         //
         // Non-string defaults.
         //
         assert(
            defval.is<gw::constant::floating_point>() ||
            defval.is<gw::constant::integer>()
         );
         auto src_value = defval.as<gw::value>();
         return expr_pair(
            gw::expr::assign(*value.read, src_value),
            gw::expr::base::wrap(build_empty_stmt(UNKNOWN_LOCATION))
         );
      }
      
      //
      // Handle values that aren't omitted-and-defaulted.
      //
      
      if (options.is<typed_options::boolean>()) {
         return expr_pair(
            gw::expr::assign(
               *value.read,
               gw::expr::call(
                  *global.functions.read.boolean,
                  // args:
                  *ctxt.state_ptr.read
               )
            ),
            gw::expr::call(
               *global.functions.save.boolean,
               // args:
               *ctxt.state_ptr.save,
               *value.save
            )
         );
      }
      
      if (options.is<typed_options::buffer>()) {
         auto bytecount = options.as<typed_options::buffer>().bytecount;
         auto size_arg  = gw::constant::integer(ty.uint16, bytecount);
         return expr_pair(
            gw::expr::call(
               *global.functions.read.buffer,
               // args:
               *ctxt.state_ptr.read,
               value.read->address_of(),
               size_arg
            ),
            gw::expr::call(
               *global.functions.save.buffer,
               // args:
               *ctxt.state_ptr.save,
               value.save->address_of(),
               size_arg
            )
         );
      }
      
      if (options.is<typed_options::integral>()) {
         auto& int_opt = options.as<typed_options::integral>();
         
         auto type = value.read->value_type();
         {
            // If the value we're working with is a bitfield, then we need to 
            // get the non-bitfielded type.
         }
         gw::decl::optional_function read_func;
         gw::decl::optional_function save_func;
         {
            auto vt_canonical = type.canonical();
            //
            // Because we add a `min` to the value before serializing, 
            // we should basically never use the signed functions.
            //
            /*//
            if (vt_canonical == ty.uint8 || vt_canonical == ty.basic_char) {
               read_func = global.functions.read.u8;
               save_func = global.functions.save.u8;
            } else if (vt_canonical == ty.uint16) {
               read_func = global.functions.read.u16;
               save_func = global.functions.save.u16;
            } else if (vt_canonical == ty.uint32) {
               read_func = global.functions.read.u32;
               save_func = global.functions.save.u32;
            } else if (vt_canonical == ty.int8) {
               read_func = global.functions.read.s8;
               save_func = global.functions.save.s8;
            } else if (vt_canonical == ty.int16) {
               read_func = global.functions.read.s16;
               save_func = global.functions.save.s16;
            } else if (vt_canonical == ty.int32) {
               read_func = global.functions.read.s32;
               save_func = global.functions.save.s32;
            }
            //*/
            size_t bitcount = 0;
            if (vt_canonical == ty.uint8 || vt_canonical == ty.int8 || vt_canonical == ty.basic_char) {
               bitcount = 8;
            } else if (vt_canonical == ty.uint16 || vt_canonical == ty.int16) {
               bitcount = 16;
            } else if (vt_canonical == ty.uint32 || vt_canonical == ty.int32) {
               bitcount = 32;
            } else if (vt_canonical == ty.basic_int) {
               bitcount = ty.basic_int.bitcount();
            } else {
               //
               // Bitfield type or unsupported type.
               //
               bitcount = vt_canonical.size_in_bits();
            }
            if (bitcount <= 8) {
               read_func = global.functions.read.u8;
               save_func = global.functions.save.u8;
            } else if (bitcount <= 16) {
               read_func = global.functions.read.u16;
               save_func = global.functions.save.u16;
            } else if (bitcount <= 32) {
               read_func = global.functions.read.u32;
               save_func = global.functions.save.u32;
            } else {
               assert(bitcount != 0 && "Unknown integral type!");
               assert(false && "The `int` type has an unexpected/unsupported bitcount!");
            }
         }
         assert(!!read_func);
         assert(!!save_func);
         
         auto ic_bitcount = gw::constant::integer(ty.uint8,           int_opt.bitcount);
         auto ic_min      = gw::constant::integer(type.as_integral(), int_opt.min);
         
         optional_expr_pair out;
         {  // Read
            gw::value to_assign = gw::expr::call(
               *read_func,
               // args:
               *ctxt.state_ptr.read,
               ic_bitcount
            );
            if (int_opt.min != 0 && int_opt.min != typed_options::integral::no_minimum) {
               //
               // If a field's range of valid values is [a, b], then serialize it as (v - a), 
               // and then unpack it as (v + a). This means we don't need a sign bit when we 
               // serialize negative numbers, and it also means that ranges that start above 
               // zero don't waste the low bits.
               //
               to_assign = to_assign.add(ic_min);
            }
            out.read = gw::expr::assign(*value.read, to_assign);
         }
         {  // Save
            auto to_save = *value.save;
            if (int_opt.min != 0 && int_opt.min != typed_options::integral::no_minimum)
               to_save = to_save.sub(ic_min);
            out.save = gw::expr::call(
               *save_func,
               // args:
               *ctxt.state_ptr.save,
               to_save,
               ic_bitcount
            );
         }
         return out;
      }
      
      if (options.is<typed_options::pointer>()) {
         auto type = value.read->value_type();
         gw::decl::optional_function read_func;
         gw::decl::optional_function save_func;
         switch (type.size_in_bits()) {
            case 8:
               read_func = global.functions.read.u8;
               save_func = global.functions.save.u8;
               break;
            case 16:
               read_func = global.functions.read.u16;
               save_func = global.functions.save.u16;
               break;
            case 32:
               read_func = global.functions.read.u32;
               save_func = global.functions.save.u32;
               break;
            default:
               assert(false && "unsupported pointer size");
         }
         assert(!!read_func);
         assert(!!save_func);
         
         auto bitcount_arg = gw::constant::integer(ty.uint8, type.size_in_bits());
         
         return expr_pair(
            gw::expr::assign(
               *value.read,
               gw::expr::call(
                  *read_func,
                  // args:
                  *ctxt.state_ptr.read,
                  bitcount_arg
               ).conversion_sans_bytecode(type)
            ),
            gw::expr::call(
               *save_func,
               // args:
               *ctxt.state_ptr.save,
               value.save->conversion_sans_bytecode(ty.smallest_integral_for(type.size_in_bits(), false)),
               bitcount_arg
            )
         );
      }
      
      if (options.is<typed_options::string>()) {
         auto& str_opt = options.as<typed_options::string>();
         
         gw::decl::optional_function read_func;
         gw::decl::optional_function save_func;
         if (str_opt.nonstring) {
            read_func = global.functions.read.string_ut;
            save_func = global.functions.save.string_ut;
         } else {
            read_func = global.functions.read.string_nt;
            save_func = global.functions.save.string_nt;
         }
         assert(!!read_func);
         assert(!!save_func);
         
         auto length_arg = gw::constant::integer(ty.uint16, str_opt.length);
         return expr_pair(
            gw::expr::call(
               *read_func,
               // args:
               *ctxt.state_ptr.read,
               value.read->convert_array_to_pointer(),
               length_arg
            ),
            gw::expr::call(
               *save_func,
               // args:
               *ctxt.state_ptr.save,
               value.save->convert_array_to_pointer(),
               length_arg
            )
         );
      }
      
      if (options.is<typed_options::structure>()) {
         auto type = value.read->value_type();
         assert(type.is_record());
         
         auto func = ctxt.get_whole_struct_functions_for(type.as_record());
         
         return expr_pair(
            gw::expr::call(
               func.read,
               // args:
               *ctxt.state_ptr.read,
               value.read->address_of()
            ),
            gw::expr::call(
               func.save,
               // args:
               *ctxt.state_ptr.save,
               value.save->address_of()
            )
         );
      }
      
      assert(false && "unreachable");
   }
}