#include "pragma_handlers/serialized_sector_id_to_constant.h"
#include <inttypes.h> // PRIu64
#include "gcc_wrappers/constant/integer.h"
#include "gcc_wrappers/decl/variable.h"
#include "gcc_wrappers/builtin_types.h"
#include "gcc_wrappers/identifier.h"
#include "last_generation_result.h"
#include "pragma_parse_exception.h"
#include "pragma_handlers/helpers/parse_c_serialization_item.h"
#include <c-family/c-common.h> // lookup_name
#include <diagnostic.h>
namespace gw {
   using namespace gcc_wrappers;
}

namespace pragma_handlers {
   extern void serialized_sector_id_to_constant(cpp_reader* reader) {
      constexpr const char* this_pragma_name = "#pragma lu_bitpack serialized_sector_id_to_constant";
      
      auto& result = last_generation_result::get();
      if (result.empty()) {
         error("%s: no code has been generated yet", this_pragma_name);
         return;
      }
      
      location_t pragma_loc = UNKNOWN_LOCATION;
      
      // Extract the identifier to use for the generated variable.
      gw::optional_identifier     requested_name;
      gw::decl::optional_variable generated_variable;
      {
         tree       data;
         location_t loc;
         auto token_type = pragma_lex(&data, &loc);
         pragma_loc = loc;
         if (token_type != CPP_NAME) {
            error_at(loc, "expected an identifier naming the variable to generate");
            return;
         }
         requested_name = gw::identifier::wrap(data);
         
         auto raw = lookup_name(requested_name.unwrap());
         if (raw != NULL_TREE) {
            if (TREE_CODE(raw) != VAR_DECL) {
               error_at(loc, "identifier %qE already exists, and does not name a variable", requested_name.unwrap());
               return;
            }
            generated_variable = gw::decl::variable::wrap(raw);
            
            auto type = generated_variable->value_type();
            if (!type.is_integer()) {
               error_at(loc, "identifier %qE already exists, and names a variable that is not of an integer type", requested_name.unwrap());
               return;
            }
            if (generated_variable->initial_value()) {
               error_at(loc, "identifier %qE already exists, and names a variable that is already defined", requested_name.unwrap());
               return;
            }
         }
      }
      
      // Figure out what to-be-serialized value the user wants to know about.
      codegen::serialization_item requested_item;
      try {
         requested_item = helpers::parse_c_serialization_item(reader);
      } catch (const pragma_parse_exception& ex) {
         error_at(ex.location, ex.what());
         return;
      }
      
      // Find what sector the to-be-serialized value is in.
      auto value = result.find_containing_sector(requested_item);
      
      // Generate the variable.
      bool variable_already_declared = !!generated_variable;
      if (!generated_variable) {
         generated_variable = gw::decl::variable(*requested_name, gw::builtin_types::get().size, pragma_loc);
         generated_variable->make_artificial();
         generated_variable->make_read_only();
      }
      {
         auto type = generated_variable->value_type();
         if (!value.has_value()) {
            error_at(pragma_loc, "%qs: the requested value does not appear to be present in the bitstream", this_pragma_name);
            //
            // We create the variable even if we don't have a match, so that user 
            // code which refers to the variable doesn't generate compiler errors 
            // about its nonexistence.
            //
            value = 0;
         }
         auto cst = gw::constant::integer(type.as_integral(), *value);
         generated_variable->set_initial_value(cst);
      }
      if (!variable_already_declared) {
         generated_variable->commit_to_current_scope();
         generated_variable->set_is_defined_elsewhere(false);
      }
      
      // Report status to the user.
      if (variable_already_declared) {
         inform(pragma_loc, "generated value %" PRIu64 " for previously-declared variable %qE", (uint64_t)*value, requested_name.unwrap());
      } else {
         inform(pragma_loc, "generated variable %qE with value %" PRIu64, requested_name.unwrap(), (uint64_t)*value);
      }
   }
}