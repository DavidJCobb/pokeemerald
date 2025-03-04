#include <array>
#include <cassert>
#include <memory> // GCC headers fuck up string-related STL identifiers that <memory> depends on
#include <string>
#include <string_view>
#include <vector>
#include "lu/stringf.h"
#include "pragma_handlers/generate_functions.h"

#include <tree.h>
#include <c-family/c-common.h> // lookup_name
#include <stringpool.h> // get_identifier
#include <c-tree.h> // c_bind
#include <toplev.h> // rest_of_decl_compilation
#include <diagnostic.h>

#include "codegen/generation_request.h"
#include "codegen/generation_result.h"

#include "basic_global_state.h"
#include "last_generation_result.h"
#include "codegen/debugging/print_sectored_serialization_items.h"
#include "codegen/debugging/print_sectored_rechunked_items.h"
#include "codegen/instructions/base.h"
#include "codegen/serialization_item_list_ops/divide_items_by_sectors.h"
#include "codegen/serialization_item_list_ops/get_total_serialized_size.h"
#include "codegen/rechunked/item.h"
#include "codegen/rechunked/items_to_instruction_tree.h"
#include "codegen/decl_descriptor.h"
#include "codegen/decl_dictionary.h"
#include "codegen/describe_and_check_decl_tree.h"
#include "codegen/serialization_item.h"

#include "gcc_helpers/c/at_file_scope.h"
#include "gcc_helpers/stringify_function_signature.h"
#include "gcc_wrappers/constant/integer.h"
#include "gcc_wrappers/decl/base.h"
#include "gcc_wrappers/decl/function.h"
#include "gcc_wrappers/decl/variable.h"
#include "gcc_wrappers/environment/c/constexpr_supported.h"
#include "gcc_wrappers/environment/c/dialect.h"
#include "gcc_wrappers/type/base.h"
#include "gcc_wrappers/type/function.h"
#include "gcc_wrappers/builtin_types.h"
namespace gw {
   using namespace gcc_wrappers;
}

// for generating XML output:
#include "codegen/stats_gatherer.h"
#include "xmlgen/report_generator.h"
#include <fstream>

namespace pragma_handlers {
   // Returns true if valid; false if error.
   static bool _check_and_report_missing_builtin(
      location_t       loc,
      gw::decl::optional_function decl,
      std::string_view name,
      std::string_view header
   ) {
      if (decl)
         return true;
      
      error_at(loc, "%<#pragma lu_bitpack generate_functions%>: unable to find a definition for %<%s%> or %<__builtin_%s%>, which we may need for code generation", name.data(), name.data());
      inform(loc, "%qs would be absent if %s has not been included yet; and %<__builtin_%s%>, used a fallback, would be absent based on command line parameters passed to GCC", name.data(), header.data(), name.data());
      return false;
   }
   
   template<size_t ArgCount>
   static bool _check_and_report_builtin_signature(
      location_t     loc,
      gw::decl::optional_function decl,
      gw::type::base return_type,
      std::array<gw::type::base, ArgCount> args
   ) {
      if (!decl)
         return true;
      auto type  = decl->function_type();
      bool valid = type.has_signature(
         false, // not unprototyped
         false, // not varargs
         true,  // require exact match; do not consider implicit argument conversions
         return_type,
         args
      );
      if (!valid) {
         error_at(loc, "%<#pragma lu_bitpack generate_functions%>: function %<%s%>, which we may need for code generation, has an unexpected signature", decl->name().data());
         
         {
            auto str = gcc_helpers::stringify_function_signature(
               false,
               return_type,
               decl->name(),
               args
            );
            inform(loc, "expected signature was %<%s%>", str.c_str());
         }
         {
            auto str = gcc_helpers::stringify_function_signature(*decl);
            inform(loc, "signature seen is %<%s%>", str.c_str());
            if constexpr (ArgCount == 0) {
               if (type.is_unprototyped()) {
                 inform(loc, "prior to C23, the syntax %<void f()%> defines an unprototyped (pre-C89) function declaration that can take any amount of arguments, as opposed to %<void f(void)%>, which would define a function that takes no arguments"); 
               }
            }
         }
      }
      return valid;
   }
   
   extern void generate_functions(cpp_reader* reader) {
   
      // force-create the singleton now, so other code can safely use `get_fast` 
      // to access it.
      gw::builtin_types::get();
      
      codegen::generation_request request;
      if (!request.from_pragma_parser(*reader))
         return;
      
      bool bad_generated_function_names = !request.are_function_names_valid();
      if (!gcc_helpers::c::at_file_scope()) {
         //
         // GCC name lookups are relative to the current scope. AFAIK there's 
         // no way to only do lookups within file scope. This means that if we 
         // allow this pragma to be used in any inner scope, the identifiers 
         // that the user specified in the pragma options may match function 
         // locals -- and we'll generate file-scope functions that refer to 
         // another function's locals, causing GCC to crash when compiling the 
         // code we generate.
         //
         error_at(request.start_location, "%<#pragma lu_bitpack generate_functions%> must be used at file scope (i.e. outside of any function, struct definition, and similar)");
         return;
      }
      
      auto& gs = basic_global_state::get();
      if (gs.any_attributes_missed) {
         error_at(request.start_location, "%<#pragma lu_bitpack generate_functions%>: in order to run code generation, you must use %<#pragma lu_bitpack enable%> before the compiler sees any bitpacking attributes");
         
         auto first = gs.error_locations.first_missed_attribute;
         if (first != UNKNOWN_LOCATION)
            inform(first, "the first seen bitpacking attribute was here");
         
         return;
      }
      if (gs.global_options.invalid) {
         error_at(request.start_location, "%<#pragma lu_bitpack generate_functions%>: unable to generate code due to a previous error in %<#pragma lu_bitpack set_options%>");
         return;
      }
      
      bool has_builtins      = true;
      bool identifiers_valid = false;
      if (gcc_helpers::c::at_file_scope()) {
         identifiers_valid = request.are_identifiers_valid();
         
         gs.pull_builtin_functions();
         if (!_check_and_report_missing_builtin(request.start_location, gs.builtin_functions.memcpy, "memcpy", "<string.h>")) {
            has_builtins = false;
         } else {
            const auto& ty = gw::builtin_types::get_fast();
            if (!_check_and_report_builtin_signature(
               request.start_location,
               *gs.builtin_functions.memcpy,
               ty.void_ptr,
               std::array<gw::type::base, 3>{
                  ty.void_ptr,
                  ty.const_void_ptr,
                  ty.size
               }
            )) {
               has_builtins = false;
            }
         }
         if (!_check_and_report_missing_builtin(request.start_location, gs.builtin_functions.memset, "memset", "<string.h>")) {
            has_builtins = false;
         } else {
            const auto& ty = gw::builtin_types::get_fast();
            if (!_check_and_report_builtin_signature(
               request.start_location,
               *gs.builtin_functions.memset,
               ty.void_ptr,
               std::array<gw::type::base, 3>{
                  ty.void_ptr,
                  ty.basic_int,
                  ty.size
               }
            )) {
               has_builtins = false;
            }
         }
      }
      
      if (bad_generated_function_names || !identifiers_valid || !has_builtins) {
         error_at(request.start_location, "%<#pragma lu_bitpack generate_functions%>: aborting codegen");
         return;
      }
      
      auto& decl_dictionary = codegen::decl_dictionary::get();
      
      //
      // Generate serialization items for all to-be-serialized values, and 
      // split items into and across sectors as appropriate.
      //
      std::vector<std::vector<codegen::serialization_item>> all_sectors_si;
      {
         size_t sector_size_in_bits = gs.global_options.sectors.bytes_per * 8;
         
         for(auto& group : request.identifier_groups) {
            std::vector<codegen::serialization_item> items;
            for(auto& entry : group) {
               auto id   = entry.id;
               auto decl = gw::decl::variable::wrap(lookup_name(id.unwrap()));
               
               const auto& desc =
                  entry.dereference_count > 0 ?
                     decl_dictionary.dereference_and_describe(decl, entry.dereference_count)
                  :
                     decl_dictionary.describe(decl)
               ;
               if (!codegen::describe_and_check_decl_tree(desc)) {
                  error_at(entry.loc, "%<#pragma lu_bitpack generate_functions%>: aborting codegen due to invalid bitpacking data options applied to a to-be-serialized value");
                  return;
               }
               
               auto& item = items.emplace_back();
               auto& segm = item.segments.emplace_back();
               auto& data = segm.data.emplace<codegen::serialization_items::basic_segment>();
               data.desc = &desc;
            }
            
            {
               auto these_sectors = codegen::serialization_item_list_ops::divide_items_by_sectors(sector_size_in_bits, items);
               for(size_t i = 0; i < these_sectors.size(); ++i) {
                  all_sectors_si.push_back(std::move(these_sectors[i]));
               }
            }
            //
            // Verify that we fit under the sector count limit; if we don't, 
            // report the problem to the user, show any pending debug output, 
            // and then abort.
            //
            if (all_sectors_si.size() > gs.global_options.sectors.max_count) {
               if (request.settings.enable_debug_output) {
                  codegen::debugging::print_sectored_serialization_items(all_sectors_si);
               }
               error(
                  "%<#pragma lu_bitpack generate_functions%>: code generation was limited to %u sectors, but the data requires %u sectors",
                  (int)gs.global_options.sectors.max_count,
                  (int)all_sectors_si.size()
               );
               
               if (gs.global_options.sectors.max_count > 0) {
                  //
                  // Try to report the last value that fit.
                  //
                  auto last_sector_index = gs.global_options.sectors.max_count - 1;
                  
                  const codegen::serialization_item* last_non_padding = nullptr;
                  auto& sector = all_sectors_si[last_sector_index];
                  for(auto it = sector.rbegin(); it != sector.rend(); ++it) {
                     auto& item = *it;
                     if (item.is_padding())
                        continue;
                     last_non_padding = &item;
                     break;
                  }
                  if (last_non_padding) {
                     std::string value;
                     for(auto& segm : last_non_padding->segments) {
                        auto& data = segm.as_basic();
                        if (!value.empty())
                           value += '.';
                        value += data.desc->decl.name();
                        for(auto& aai : data.array_accesses) {
                           if (aai.count == 1)
                              value += lu::stringf("[%u]", aai.start);
                           else
                              value += lu::stringf("[%u:%u]", aai.start, aai.start + aai.count);
                        }
                     }
                     inform(UNKNOWN_LOCATION, "the last value that fit into sector %u was %qs", (int)last_sector_index, value.c_str());
                  } else {
                     inform(UNKNOWN_LOCATION, "sector %u is filled entirely with padding, used to even out the size of tagged unions%' members", (int)last_sector_index);
                  }
               }
               return;
            }
         }
      }
      if (request.settings.enable_debug_output) {
         codegen::debugging::print_sectored_serialization_items(all_sectors_si);
      }
      //
      // Convert serialization items to rechunked items.
      //
      std::vector<std::vector<codegen::rechunked::item>> all_sectors_ri;
      for(const auto& sector : all_sectors_si) {
         auto& dst = all_sectors_ri.emplace_back();
         for(const auto& item : sector) {
            dst.emplace_back(item);
         }
      }
      if (request.settings.enable_debug_output) {
         codegen::debugging::print_sectored_rechunked_items(all_sectors_ri);
      }
      //
      // Generate node trees.
      //
      std::vector<std::unique_ptr<codegen::instructions::base>> instructions_by_sector;
      for(const auto& sector : all_sectors_ri) {
         auto node_ptr = codegen::rechunked::items_to_instruction_tree(sector);
         instructions_by_sector.push_back(std::move(node_ptr));
      }
      
      //
      // Generate functions.
      //
      
      codegen::generation_result result;
      if (!result.generate(request, instructions_by_sector))
         return;
      
      inform(UNKNOWN_LOCATION, "generated the serialization functions");
      {  // Define file-scoped variables.
         size_t count    = gs.global_options.sectors.max_count;
         size_t max_size = gs.global_options.sectors.bytes_per;
         if (count == 1 && max_size == std::numeric_limits<size_t>::max()) {
            max_size = 0;
            if (!all_sectors_si.empty()) {
               max_size = codegen::serialization_item_list_ops::get_total_serialized_size(all_sectors_si[0]);
            }
         }
         
         const auto& ty = gw::builtin_types::get_fast();
         auto const_size_type = ty.size.add_const();
         {  // __lu_bitpack_sector_count
            gw::decl::variable var("__lu_bitpack_sector_count", const_size_type);
            var.make_artificial();
            var.set_initial_value(gw::constant::integer(ty.size, count));
            var.make_read_only();
            var.make_file_scope_extern();
            var.set_is_defined_elsewhere(false);
            if constexpr (gw::environment::c::constexpr_supported) {
               if (gw::environment::c::current_dialect() >= gw::environment::c::dialect::c23) {
                  var.make_declared_constexpr();
               }
            }
         }
         {  // __lu_bitpack_max_sector_size
            gw::decl::variable var("__lu_bitpack_max_sector_size", const_size_type);
            var.make_artificial();
            var.set_initial_value(gw::constant::integer(ty.size, max_size));
            var.make_read_only();
            var.make_file_scope_extern();
            var.set_is_defined_elsewhere(false);
            if constexpr (gw::environment::c::constexpr_supported) {
               if (gw::environment::c::current_dialect() >= gw::environment::c::dialect::c23) {
                  var.make_declared_constexpr();
               }
            }
         }
         inform(UNKNOWN_LOCATION, "generated built-in variables (%<__lu_bitpack_sector_count%> and friends)");
      }
      
      {
         auto& dst = last_generation_result::get();
         dst.update(all_sectors_si);
      }
      
      //
      // Produce XML output, if possible.
      //
      if (!gs.xml_output_path.empty()) {
         const auto& path = gs.xml_output_path;
         if (path.ends_with(".xml")) {
            codegen::stats_gatherer stats;
            stats.gather_from_sectors(all_sectors_si);
            
            xmlgen::report_generator xml_gen;
            xml_gen.process(request);
            xml_gen.process(result.whole_struct);
            for(const auto& node_ptr : instructions_by_sector)
               xml_gen.process(*node_ptr->as<codegen::instructions::container>());
            xml_gen.process(stats);
            
            std::ofstream stream(path.c_str());
            assert(!!stream);
            stream << xml_gen.bake();
         }
      }
   }
}