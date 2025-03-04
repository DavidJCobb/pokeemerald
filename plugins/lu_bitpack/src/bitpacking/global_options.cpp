#include "bitpacking/global_options.h"
#include "lu/stringf.h"
#include "bitpacking/requested_global_options.h"
#include "gcc_wrappers/type/helpers/lookup_by_name.h"
#include "gcc_wrappers/builtin_types.h"
#include <c-family/c-common.h> // lookup_name
#include <diagnostic.h>
namespace gw {
   using namespace gcc_wrappers;
}

namespace bitpacking {
   void global_options::_missing_option(const requested_global_options& src, std::string_view name) {
      error_at(src.pragma_location, "option %<%s%> was not specified", name.data());
      this->invalid = true;
   }

   gw::decl::optional_function global_options::_get_function_or_fail(location_t loc, gw::identifier id) {
      auto raw  = lookup_name(id.unwrap());
      if (raw == NULL_TREE) {
         error_at(loc, "identifier %qE is not defined here", id.unwrap());
         this->invalid = true;
      } else if (!gw::decl::function::raw_node_is(raw)) {
         error_at(loc, "identifier %qE names something other than a function", id.unwrap());
         this->invalid = true;
      } else {
         return gw::decl::function::wrap(raw);
      }
      return {};
   }
   
   bool global_options::_check_and_report_unprototyped(location_t loc, gw::type::function type) {
      if (type.is_unprototyped()) {
         error_at(loc, "the specified function is unprototyped; this is wrong");
         this->invalid = true;
         return true;
      }
      return false;
   }
   
   void global_options::grab(const requested_global_options& src) {
      const auto& ty = gw::builtin_types::get_fast();
      this->invalid = false;
      
      //
      // Sector options:
      //
      
      {
         auto& opt = src.sectors.max_count;
         auto& dst = this->sectors.max_count;
         if (opt.has_value())
            dst = (*opt).data;
         else
            dst = 1;
      }
      {
         auto& opt = src.sectors.bytes_per;
         auto& dst = this->sectors.bytes_per;
         if (opt.has_value())
            dst = (*opt).data;
         else
            dst = std::numeric_limits<size_t>::max();
      }
      
      //
      // Type options:
      //
      
      if (auto& opt = src.types.boolean; opt.has_value()) {
         auto id   = opt->data;
         auto type = gw::type::lookup_by_name(id);
         if (!type) {
            error_at(opt->loc.data, "identifier %qE does not name a type", id.unwrap());
            this->invalid = true;
         } else if (!type->is_integral()) {
            error_at(opt->loc.data, "identifier %qE names a non-integral type", id.unwrap());
            this->invalid = true;
         } else {
            this->types.boolean = type;
         }
      } else {
         this->types.boolean = ty.basic_bool;
      }
      
      if (auto& opt = src.types.bitstream_state; opt.has_value()) {
         auto id   = opt->data;
         auto type = gw::type::lookup_by_name(id);
         if (!type) {
            error_at(opt->loc.data, "identifier %qE does not name a type", id.unwrap());
            this->invalid = true;
         } else if (!type->is_record()) {
            error_at(opt->loc.data, "identifier %qE names a non-struct type", id.unwrap());
            this->invalid = true;
         } else {
            this->types.bitstream_state     = type->as_record();
            this->types.bitstream_state_ptr = type->add_pointer();
         }
      } else {
         _missing_option(src, "bitstream_state_typename");
      }
      
      if (auto& opt = src.types.buffer_byte; opt.has_value()) {
         auto id   = opt->data;
         auto type = gw::type::lookup_by_name(id);
         if (!type) {
            error_at(opt->loc.data, "identifier %qE does not name a type", id.unwrap());
            this->invalid = true;
         } else {
            this->types.buffer_byte     = type;
            this->types.buffer_byte_ptr = type->add_pointer();
         }
      } else {
         _missing_option(src, "buffer_byte_typename");
      }
      
      //
      // Resolve functions:
      //
      
      // void lu_BitstreamInitialize(struct lu_BitstreamState*, uint8_t* buffer)
      if (auto& opt = src.functions.stream_state_init; opt.has_value()) {
         auto loc  = opt->loc.data;
         auto fopt = _get_function_or_fail(loc, opt->data);
         if (fopt && this->types.bitstream_state_ptr && this->types.buffer_byte_ptr) {
            auto decl = *fopt;
            auto type = decl.function_type();
            if (type.has_signature(
               false,
               false,
               true,
               ty.basic_void,
               std::array<gw::type::base, 2>{
                  *this->types.bitstream_state_ptr,
                  *this->types.buffer_byte_ptr,
               }
            )) {
               this->functions.stream_state_init = decl;
            } else {
               error_at(
                  loc,
                  "the specified function has the wrong signature (expected: %<void %s(struct %s*, %s*)%>)",
                  decl.name().data(),
                  this->types.bitstream_state->name().data(),
                  this->types.buffer_byte->name().data()
               );
               this->invalid = true;
            }
         }
      } else {
         _missing_option(src, "func_initialize");
      }
      
      // bitstream read
      {
         constexpr const char* operation = "read";
         auto& src_group = src.functions.read;
         auto& dst_group = this->functions.read;
         
         // bool lu_BitstreamRead_bool(struct lu_BitstreamState*)
         if (auto& opt = src_group.boolean; opt.has_value()) {
            auto loc  = opt->loc.data;
            auto fopt = _get_function_or_fail(loc, opt->data);
            if (fopt && this->types.bitstream_state_ptr && this->types.boolean) {
               auto decl = *fopt;
               auto type = decl.function_type();
               if (type.has_signature(
                  false,
                  false,
                  true,
                  *this->types.boolean,
                  std::array<gw::type::base, 1>{
                     *this->types.bitstream_state_ptr,
                  }
               )) {
                  dst_group.boolean = decl;
               } else {
                  error_at(
                     loc,
                     "the specified function has the wrong signature (expected: %<%s %s(struct %s*)%>)",
                     this->types.boolean->name().data(),
                     decl.name().data(),
                     this->types.bitstream_state->name().data()
                  );
                  this->invalid = true;
               }
            }
         } else {
            _missing_option(src, lu::stringf("func_%s_bool", operation));
         }
         
         auto _get_int_function = [this, &src, &ty](
            const char*    variant_name,
            gw::type::base variant_type,
            const std::optional<requested_global_options::identifier_option>& opt,
            gw::decl::optional_function& dst
         ) {
            if (opt.has_value()) {
               auto loc  = opt->loc.data;
               auto fopt = _get_function_or_fail(loc, opt->data);
               if (fopt && this->types.bitstream_state_ptr) {
                  auto decl = *fopt;
                  auto type = decl.function_type();
                  if (type.has_signature(
                     false,
                     false,
                     true,
                     variant_type,
                     std::array<gw::type::base, 2>{
                        *this->types.bitstream_state_ptr,
                        ty.uint8
                     }
                  )) {
                     dst = decl;
                  } else {
                     error_at(
                        loc,
                        "the specified function has the wrong signature (expected: %<%s %s(struct %s*, uint8_t bitcount)%>)",
                        variant_type.name().data(),
                        decl.name().data(),
                        this->types.bitstream_state->name().data()
                     );
                     this->invalid = true;
                  }
               }
            } else {
               _missing_option(src, lu::stringf("func_%s_%s", operation, variant_name));
            }
         };
         //
         _get_int_function("s8",  ty.int8,  src_group.s8,  dst_group.s8);
         _get_int_function("s16", ty.int16, src_group.s16, dst_group.s16);
         _get_int_function("s32", ty.int32, src_group.s32, dst_group.s32);
         //
         _get_int_function("u8",  ty.uint8,  src_group.u8,  dst_group.u8);
         _get_int_function("u16", ty.uint16, src_group.u16, dst_group.u16);
         _get_int_function("u32", ty.uint32, src_group.u32, dst_group.u32);
         
         auto _get_buffer_like_function = [this, &src, &ty](
            const char* variant_name,
            const std::optional<requested_global_options::identifier_option>& opt,
            gw::decl::optional_function& dst
         ) {
            if (opt.has_value()) {
               auto loc  = opt->loc.data;
               auto fopt = _get_function_or_fail(loc, opt->data);
               if (fopt && this->types.bitstream_state_ptr) {
                  auto decl = *fopt;
                  auto type = decl.function_type();
                  
                  bool signature_matches = [this, type, &ty]() {
                     if (type.is_unprototyped())
                        return false;
                     if (type.is_varargs())
                        return false;
                     if (type.fixed_argument_count() != 3)
                        return false;
                     
                     if (type.nth_argument_type(0) != *this->types.bitstream_state_ptr)
                        return false;
                     
                     auto arg = type.nth_argument_type(1).canonical();
                     if (!arg.is_pointer())
                        return false;
                     arg = arg.remove_pointer();
                     if (!arg.is_void() && (!arg.is_integer() || arg.size_in_bytes() != 1))
                        return false;
                     if (arg.is_const())
                        return false;
                     
                     if (type.nth_argument_type(2) != ty.uint16)
                        return false;
                     
                     return true;
                  }();
                  
                  if (signature_matches) {
                     dst = decl;
                  } else {
                     error_at(
                        loc,
                        "the specified function has the wrong signature (expected: %<void %s(struct %s*, void*, u16 length)%>, or %<void %s(struct %s*, T*, u16 length)%> given some single-byte integral type %<T%>)",
                        decl.name().data(),
                        this->types.bitstream_state->name().data(),
                        decl.name().data(),
                        this->types.bitstream_state->name().data()
                     );
                     this->invalid = true;
                  }
               }
            } else {
               _missing_option(src, lu::stringf("func_%s_%s", operation, variant_name));
            }
         };
         //
         _get_buffer_like_function("string_nt", src_group.string_nt, dst_group.string_nt);
         _get_buffer_like_function("string_ut", src_group.string_ut, dst_group.string_ut);
         _get_buffer_like_function("buffer",    src_group.buffer,    dst_group.buffer);
      }
      // bitstream read -- end
      
      // bitstream write
      {
         constexpr const char* operation = "write";
         auto& src_group = src.functions.save;
         auto& dst_group = this->functions.save;
         
         // void lu_BitstreamWrite_bool(struct lu_BitstreamState*, bool)
         if (auto& opt = src_group.boolean; opt.has_value()) {
            auto loc  = opt->loc.data;
            auto fopt = _get_function_or_fail(loc, opt->data);
            if (fopt && this->types.bitstream_state_ptr && this->types.boolean) {
               auto decl = *fopt;
               auto type = decl.function_type();
               if (type.has_signature(
                  false,
                  false,
                  true,
                  ty.basic_void,
                  std::array<gw::type::base, 2>{
                     *this->types.bitstream_state_ptr,
                     *this->types.boolean
                  }
               )) {
                  dst_group.boolean = decl;
               } else {
                  error_at(
                     loc,
                     "the specified function has the wrong signature (expected: %<void %s(struct %s*, %s)%>)",
                     decl.name().data(),
                     this->types.bitstream_state->name().data(),
                     this->types.boolean->name().data()
                  );
                  this->invalid = true;
               }
            }
         } else {
            _missing_option(src, lu::stringf("func_%s_bool", operation));
         }
         
         auto _get_int_function = [this, &src, &ty](
            const char*    variant_name,
            gw::type::base variant_type,
            const std::optional<requested_global_options::identifier_option>& opt,
            gw::decl::optional_function& dst
         ) {
            if (opt.has_value()) {
               auto loc  = opt->loc.data;
               auto fopt = _get_function_or_fail(loc, opt->data);
               if (fopt && this->types.bitstream_state_ptr) {
                  auto decl = *fopt;
                  auto type = decl.function_type();
                  if (type.has_signature(
                     false,
                     false,
                     true,
                     ty.basic_void,
                     std::array<gw::type::base, 3>{
                        *this->types.bitstream_state_ptr,
                        variant_type,
                        ty.uint8
                     }
                  )) {
                     dst = decl;
                  } else {
                     error_at(
                        loc,
                        "the specified function has the wrong signature (expected: %<void %s(struct %s*, %s, uint8_t bitcount)%>)",
                        decl.name().data(),
                        this->types.bitstream_state->name().data(),
                        variant_type.name().data()
                     );
                     this->invalid = true;
                  }
               }
            } else {
               _missing_option(src, lu::stringf("func_%s_%s", operation, variant_name));
            }
         };
         //
         _get_int_function("s8",  ty.int8,  src_group.s8,  dst_group.s8);
         _get_int_function("s16", ty.int16, src_group.s16, dst_group.s16);
         _get_int_function("s32", ty.int32, src_group.s32, dst_group.s32);
         //
         _get_int_function("u8",  ty.uint8,  src_group.u8,  dst_group.u8);
         _get_int_function("u16", ty.uint16, src_group.u16, dst_group.u16);
         _get_int_function("u32", ty.uint32, src_group.u32, dst_group.u32);
         
         auto _get_buffer_like_function = [this, &src, &ty](
            const char* variant_name,
            const std::optional<requested_global_options::identifier_option>& opt,
            gw::decl::optional_function& dst
         ) {
            if (opt.has_value()) {
               auto loc  = opt->loc.data;
               auto fopt = _get_function_or_fail(loc, opt->data);
               if (fopt && this->types.bitstream_state_ptr) {
                  auto decl = *fopt;
                  auto type = decl.function_type();
                  
                  bool signature_matches = [this, type, &ty]() {
                     if (type.is_unprototyped())
                        return false;
                     if (type.is_varargs())
                        return false;
                     if (type.fixed_argument_count() != 3)
                        return false;
                     
                     if (type.nth_argument_type(0) != *this->types.bitstream_state_ptr)
                        return false;
                     
                     auto arg = type.nth_argument_type(1).canonical();
                     if (!arg.is_pointer())
                        return false;
                     arg = arg.remove_pointer();
                     if (!arg.is_void() && (!arg.is_integer() || arg.size_in_bytes() != 1))
                        return false;
                     
                     if (type.nth_argument_type(2) != ty.uint16)
                        return false;
                     
                     return true;
                  }();
                  
                  if (signature_matches) {
                     dst = decl;
                  } else {
                     error_at(
                        loc,
                        "the specified function has the wrong signature (expected: %<void %s(struct %s*, void*, u16 length)%>, or %<void %s(struct %s*, T*, u16 length)%> given some single-byte integral type %<T%>)",
                        decl.name().data(),
                        this->types.bitstream_state->name().data(),
                        decl.name().data(),
                        this->types.bitstream_state->name().data()
                     );
                     this->invalid = true;
                  }
               }
            } else {
               _missing_option(src, lu::stringf("func_%s_%s", operation, variant_name));
            }
         };
         //
         _get_buffer_like_function("string_nt", src_group.string_nt, dst_group.string_nt);
         _get_buffer_like_function("string_ut", src_group.string_ut, dst_group.string_ut);
         _get_buffer_like_function("buffer",    src_group.buffer,    dst_group.buffer);
      }
      // bitstream write -- end
      
      // Done.
   }
   
   //
   // Helpers:
   //
   
   bool global_options::type_is_boolean(const gw::type::base type) const {
      if (this->types.boolean)
         return type.is_type_or_transitive_typedef_thereof(*this->types.boolean);
      return false;
   }
}