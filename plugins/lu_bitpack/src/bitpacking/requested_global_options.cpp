#include "bitpacking/requested_global_options.h"
#include <cassert>
#include "lu/strings/zview.h"
#include "lu/stringf.h"
#include "pragma_parse_exception.h"
#include "gcc_wrappers/constant/integer.h"
#include "gcc_wrappers/constant/string.h"
#include "gcc_wrappers/identifier.h"
#include <diagnostic.h>
namespace gw {
   using namespace gcc_wrappers;
}

namespace bitpacking {
   std::optional<requested_global_options::identifier_option>* requested_global_options::_id_option_for_key(std::string_view key) {
      if (key == "sector_count" || key == "sector_size")
         return nullptr;
      
      if (key == "bitstream_state_typename")
         return &this->types.bitstream_state;
      if (key == "bool_typename")
         return &this->types.boolean;
      if (key == "buffer_byte_typename")
         return &this->types.buffer_byte;
      
      if (key.starts_with("func_")) {
         key.remove_prefix(5);
         if (key == "initialize") {
            return &this->functions.stream_state_init;
         }
         function_set* fset = nullptr;
         if (key.starts_with("read_")) {
            fset = &this->functions.read;
            key.remove_prefix(sizeof("read_") - 1);
         } else if (key.starts_with("write_")) {
            fset = &this->functions.save;
            key.remove_prefix(sizeof("write_") - 1);
         } else {
            return nullptr;
         }
         
         if (key == "bool")
            return &fset->boolean;
         if (key == "buffer")
            return &fset->buffer;
         if (key == "s8")
            return &fset->s8;
         if (key == "s16")
            return &fset->s16;
         if (key == "s32")
            return &fset->s32;
         if (key == "u8")
            return &fset->u8;
         if (key == "u16")
            return &fset->u16;
         if (key == "u32")
            return &fset->u32;
         if (key == "string" || key == "string_nt")
            return &fset->string_nt;
         if (key == "string_ut")
            return &fset->string_ut;
         
         return nullptr;
      }
      
      return nullptr;
   }
   std::optional<requested_global_options::size_option>* requested_global_options::_size_option_for_key(std::string_view key) {
      if (key == "sector_count")
         return &this->sectors.max_count;
      if (key == "sector_size")
         return &this->sectors.bytes_per;
      
      return nullptr;
   }
   
   requested_global_options::requested_global_options(cpp_reader& reader) {
      tree dummy_token = NULL_TREE;
      if (pragma_lex(&dummy_token, &this->pragma_location) != CPP_OPEN_PAREN) {
         throw pragma_parse_exception(
            this->pragma_location,
            "missing %<(%> after pragma name; pragma ignored"
         );
      }
      
      bool seen_any = false;
      do {
         std::string_view key;
         location_t       key_loc;
         tree             data;
         location_t       loc;
         
         auto token_type = pragma_lex(&data, &key_loc);
         if (token_type == CPP_CLOSE_PAREN) {
            if (seen_any) {
               warning_at(key_loc, OPT_Wpragmas, "trailing comma at end of this pragma");
            } else {
               warning_at(key_loc, OPT_Wpragmas, "you have not defined any global bitpacking options here, so bitpacking code generation will fail; is this intentional?");
            }
            break;
         }
         if (token_type != CPP_NAME) {
            throw pragma_parse_exception(
               key_loc,
               "expected key for key/value pair, after %<%c%>; pragma ignored",
               seen_any ? ',' : '('
            );
         }
         
         seen_any = true;
         key      = IDENTIFIER_POINTER(data);
         assert(!key.empty()); // how can GCC know it's an identifier if it's empty?
         
         token_type = pragma_lex(&data, &loc);
         if (token_type != CPP_EQ) {
            throw pragma_parse_exception(
               loc,
               "expected key/value separator %<=%> after key %<%s%>; pragma ignored",
               key.data()
            );
         }
         
         cpp_ttype token_after_value = (cpp_ttype) -1;
         if (auto* dst = _id_option_for_key(key)) {
            gw::optional_identifier value;
            
            token_type = pragma_lex(&data, &loc);
            if (token_type == CPP_NAME) {
               value = data;
            } else if (token_type == CPP_STRING) {
               auto               node = gw::constant::string::wrap(data);
               lu::strings::zview text = node.value().data();
               if (text.empty()) {
                  throw pragma_parse_exception(
                     loc,
                     "an empty string literal is not a valid value for key %<%s%>; pragma ignored",
                     key.data()
                  );
               }
               value = gw::identifier(text);
            } else {
               throw pragma_parse_exception(
                  loc,
                  "expected identifier or non-empty string literal as value for key %<%s%>; pragma ignored",
                  key.data()
               );
            }
            assert(!value.empty());
            
            auto& dst_v = dst->emplace(*value);
            dst_v.loc.key  = key_loc;
            dst_v.loc.data = loc;
         } else if (auto* dst = _size_option_for_key(key)) {
            token_type = pragma_lex(&data, &loc);
            if (token_type != CPP_NUMBER) {
               throw pragma_parse_exception(
                  loc,
                  "expected positive integer constant value for key %<%s%>; got something other than a number; pragma ignored",
                  key.data()
               );
            }
            if (TREE_CODE(data) != INTEGER_CST) {
               throw pragma_parse_exception(
                  loc,
                  "expected positive integer constant value for key %<%s%>; got some other kind of number instead; pragma ignored",
                  key.data()
               );
            }
            auto node = gw::constant::integer::wrap(data);
            if (node.sign() < 0) {
               throw pragma_parse_exception(
                  loc,
                  "expected positive integer constant value for key %<%s%>; got a negative integer instead; pragma ignored",
                  key.data()
               );
            }
            auto node_v = node.try_value_unsigned();
            if (!node_v.has_value()) {
               throw pragma_parse_exception(
                  loc,
                  "expected positive integer constant value for key %<%s%>; got an integer but could not read it (too large?); pragma ignored",
                  key.data()
               );
            }
            
            auto& dst_v = dst->emplace();
            dst_v.data      = *node_v;
            dst_v.loc.key  = key_loc;
            dst_v.loc.data = loc;
         } else {
            warning_at(key_loc, OPT_Wpragmas, "unrecognized key %<%s%>; ignoring value and skipping to the next %<,%> or %<)%> in the pragma", key.data());
            do {
               token_type = pragma_lex(&data, &loc);
               if (token_type == CPP_COMMA || token_type == CPP_CLOSE_PAREN) {
                  token_after_value = token_type;
                  break;
               }
               if (token_type == CPP_EOF) {
                  throw pragma_parse_exception(loc, "expected %<,%> or %<)%> but could not find either; pragma ignored");
               }
            } while (true);
         }
         
         if (token_after_value == (cpp_ttype) -1) {
            token_after_value = pragma_lex(&data, &loc);
         }
         
         if (token_after_value == CPP_CLOSE_PAREN) {
            break;
         } else if (token_after_value == CPP_COMMA) {
            continue;
         } else {
            throw pragma_parse_exception(loc, "expected %<,%> or %<)%> but got something else; pragma ignored");
         }
      } while (true);
      
      location_t loc;
      if (pragma_lex(&dummy_token, &loc) != CPP_EOF)
         warning_at(loc, OPT_Wpragmas, "junk at end of this pragma");
   }
}