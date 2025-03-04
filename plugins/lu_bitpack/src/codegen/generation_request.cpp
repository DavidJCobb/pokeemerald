#include "codegen/generation_request.h"
#include "gcc_wrappers/decl/function.h"
#include "gcc_wrappers/decl/variable.h"
#include "gcc_wrappers/scope.h"
#include <c-family/c-common.h> // lookup_name
#include <stringpool.h> // get_identifier
#include <diagnostic.h>
#include "gcc_helpers/c/at_file_scope.h"
namespace gw {
   using namespace gcc_wrappers;
}

constexpr const char* pragma_name = "#pragma lu_bitpack generate_functions";

constexpr const char* key_for_read_func = "read_name";
constexpr const char* key_for_save_func = "save_name";

namespace codegen {
   bool generation_request::from_pragma_parser(cpp_reader& parser) {
      location_t loc;
      tree       data;
      if (pragma_lex(&data, &loc) != CPP_OPEN_PAREN) {
         error_at(loc, "%qs: %<(%> expected", pragma_name);
         return false;
      }
      this->start_location = loc;
      
      do {
         std::string_view key;
         switch (pragma_lex(&data, &loc)) {
            case CPP_NAME:
               key = IDENTIFIER_POINTER(data);
               break;
            case CPP_STRING:
               key = TREE_STRING_POINTER(data);
               break;
            default:
               error_at(loc, "%qs: expected key name for key/value pair", pragma_name);
               return false;
         }
         if (pragma_lex(&data, &loc) != CPP_EQ) {
            error_at(loc, "%qs: expected key/value separator %<=%> after key %qs", pragma_name, key.data());
            return false;
         }
         
         int token;
         //
         // Each branch should end by grabbing the next token after the 
         // last accepted token. Code after the branch acts on the last 
         // grabbed token.
         //
         if (key == "enable_debug_output") {
            int value = 0;
            switch (pragma_lex(&data, &loc)) {
               case CPP_NAME:
                  {
                     std::string_view name = IDENTIFIER_POINTER(data);
                     if (name == "true") {
                        value = 1;
                     } else if (name == "false") {
                        value = 0;
                     } else {
                        error_at(loc, "%qs: expected integer literal or identifiers %<true%> or %<false%> as value for key %qs", pragma_name, key.data());
                        return false;
                     }
                  }
                  break;
               case CPP_NUMBER:
                  if (TREE_CODE(data) == INTEGER_CST) {
                     value = TREE_INT_CST_LOW(data);
                     break;
                  }
                  [[fallthrough]];
               default:
                  error_at(loc, "%qs: expected integer literal or identifiers %<true%> or %<false%> as value for key %qs", pragma_name, key.data());
                  return false;
            }
            this->settings.enable_debug_output = value != 0;
            
            token = pragma_lex(&data, &loc);
         } else if (key == key_for_read_func || key == key_for_save_func) {
            std::string_view name;
            switch (pragma_lex(&data, &loc)) {
               case CPP_NAME:
                  name = IDENTIFIER_POINTER(data);
                  break;
               case CPP_STRING:
                  name = TREE_STRING_POINTER(data);
                  break;
               default:
                  error_at(loc, "%qs: expected function name as value for key %qs", pragma_name, key.data());
                  return false;
            }
            if (key == key_for_read_func) {
               this->function_names.read = name;
            } else {
               this->function_names.save = name;
            }
            
            token = pragma_lex(&data, &loc);
         } else if (key == "data") {
            size_t deref_count = 0;
            token = pragma_lex(&data, &loc);
            while (token == CPP_MULT) {
               ++deref_count;
               token = pragma_lex(&data, &loc);
            }
            if (token != CPP_NAME) {
               error_at(loc, "%qs: identifier expected as part of value for key %qs", pragma_name, key.data());
               return false;
            }
            if (this->identifier_groups.empty()) {
               this->identifier_groups.resize(1);
            }
            do {
               this->identifier_groups.back().push_back(identifier{
                  .id  = gw::identifier::wrap(data),
                  .loc = loc,
                  .dereference_count = deref_count,
               });
               
               token = pragma_lex(&data, &loc);
               if (token == CPP_OR) {
                  //
                  // Split to next group.
                  //
                  this->identifier_groups.emplace_back();
                  //
                  // Next token.
                  //
                  token = pragma_lex(&data, &loc);
                  if (token != CPP_MULT && token != CPP_NAME) {
                     error_at(loc, "%qs: identifier expected after %<|%>, as part of value for key %qs", pragma_name, key.data());
                     return false;
                  }
               }
               deref_count = 0;
               while (token == CPP_MULT) {
                  ++deref_count;
                  token = pragma_lex(&data, &loc);
               }
               if (token == CPP_NAME) {
                  continue;
               } else if (token == CPP_COMMA || token == CPP_CLOSE_PAREN) {
                  break;
               } else {
                  error_at(loc, "%qs: expected identifier or %<|%> within value for key %qs, or %<,%> to end the value", pragma_name, key.data());
                  return false;
               }
            } while (true);
            
         } else {
            error_at(loc, "%qs: unrecognized key %qs", pragma_name, key.data());
            return false;
         }
         //
         // Handle comma and close-paren.
         //
         if (token == CPP_COMMA) {
            continue;
         }
         if (token == CPP_CLOSE_PAREN) {
            break;
         }
         error_at(loc, "%qs: expected %<,%> or %<)%> after value for key %qs", pragma_name, key.data());
         return false;
      } while (true);
      if (this->identifier_groups.empty()) {
         warning_at(this->start_location, 1, "%qs: you did not specify any data to serialize; is this intentional?", pragma_name);
      }
      return true;
   }
   
   bool generation_request::are_identifiers_valid(bool complain) {
      for (auto& group : this->identifier_groups) {
         for(auto& entry : group) {
            auto id = entry.id;
            
            gw::optional_node node = lookup_name(id.unwrap());
            if (!node) {
               if (complain) {
                  error_at(entry.loc, "%<#pragma lu_bitpack generate_functions%>: identifier %qE is not defined", id.unwrap());
               }
               return false;
            }
            if (!node->is<gw::decl::variable>()) {
               if (complain) {
                  error_at(entry.loc, "%<#pragma lu_bitpack generate_functions%>: identifier %qE does not name a variable", id.unwrap());
               }
               return false;
            }
            {
               auto decl = node->as<gw::decl::variable>();
               auto type = decl.value_type();
               for(size_t i = 0; i < entry.dereference_count; ++i) {
                  if (!type.is_pointer()) {
                     if (complain) {
                        if (entry.dereference_count == 1) {
                           error_at(entry.loc, "%<#pragma lu_bitpack generate_functions%>: identifier %qE is meant to be dereferenced once before being serialized, but it is not a pointer", id.unwrap());
                        } else {
                           error_at(entry.loc, "%<#pragma lu_bitpack generate_functions%>: identifier %qE is meant to be dereferenced %u times before being serialized, but it is not a pointer of that depth", id.unwrap(), (unsigned int)entry.dereference_count);
                        }
                     }
                     return false;
                  }
                  type = type.remove_pointer();
               }
            }
            entry.cached.referent = node;
         }
      }
      return true;
   }
   
   bool generation_request::are_function_names_valid(bool complain) const {
      auto _check_function = [complain](const std::string& name, const char* key) -> bool {
         gw::identifier    id   = gw::identifier(name.c_str());
         gw::optional_node node = lookup_name(id.unwrap());
         if (!node)
            //
            // Identifier not declared; we can declare it.
            //
            return true;
         
         gw::optional_scope scope;
         if (node->is<gw::decl::base>()) {
            scope = node->as<gw::decl::base>().context();
         } else {
            if (complain) {
               error("%qs: per the %qs key we should generate a function with identifier %qE, but that identifier is already in use by something else", pragma_name, key, id.unwrap());
            }
            return false;
         }
         if (scope && !scope->is_file_scope()) {
            //
            // Our caller is expected to report errors if we're not in file scope, 
            // so ignore identifiers from nested scopes.
            //
            return false;
         }
         if (!node->is<gw::decl::function>()) {
            if (complain) {
               error("%qs: per the %qs key we should generate a function with identifier %qE, but that identifier is already in use by something else", pragma_name, key, id.unwrap());
            }
            return false;
         }
         return true;
      };
      
      if (this->function_names.read.empty()) {
         error("%qs: missing the %qs key (the identifier of a function to generate, with signature %<void __generated_read_func(const __buffer_byte_type* src, int sector_id)%>)", pragma_name, key_for_read_func);
         return false;
      }
      if (!_check_function(this->function_names.read, key_for_read_func))
         return false;
      
      if (this->function_names.save.empty()) {
         error("%qs: missing the %qs key (the identifier of a function to generate, with signature %<void __generated_save_func(__buffer_byte_type* src, int sector_id)%>)", pragma_name, key_for_read_func);
         return false;
      }
      if (!_check_function(this->function_names.save, key_for_save_func))
         return false;
      
      return true;
   }
}