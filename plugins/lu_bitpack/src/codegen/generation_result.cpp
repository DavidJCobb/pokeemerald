#include "lu/stringf.h"
#include "codegen/generation_result.h"
#include "codegen/generation_request.h"
#include "basic_global_state.h"
#include "gcc_wrappers/constant/integer.h"
#include "gcc_wrappers/decl/function.h"
#include "gcc_wrappers/decl/variable.h"
#include "gcc_wrappers/expr/call.h"
#include "gcc_wrappers/expr/local_block.h"
#include "gcc_wrappers/expr/ternary.h"
#include "gcc_wrappers/type/function.h"
#include "gcc_wrappers/flow/simple_if_else_set.h"
#include "gcc_wrappers/builtin_types.h"
#include "gcc_wrappers/statement_list.h"
#include "gcc_wrappers/value.h"
namespace gw {
   using namespace gcc_wrappers;
}
#include "codegen/instructions/utils/generation_context.h"
#include "codegen/expr_pair.h"
#include "codegen/optional_value_pair.h"
#include <c-family/c-common.h> // lookup_name
#include <diagnostic.h>

namespace codegen {
   void generation_result::_generate_per_sector_functions(const generation_request& request, const std::vector<std::unique_ptr<instructions::base>>& instructions_by_sector) {
      const auto& gs = basic_global_state::get_fast();
      const auto& ty = gw::builtin_types::get_fast();
      
      auto per_sector_function_type = gw::type::function(
         ty.basic_void,
         // args:
         *gs.global_options.types.bitstream_state_ptr
      );
      
      for(size_t i = 0; i < instructions_by_sector.size(); ++i) {
         auto pair = func_pair(
            gw::decl::function(
               lu::stringf("__lu_bitpack_read_sector_%u", (int)i),
               per_sector_function_type
            ),
            gw::decl::function(
               lu::stringf("__lu_bitpack_save_sector_%u", (int)i),
               per_sector_function_type
            )
         );
         pair.read.as_modifiable().set_result_decl(gw::decl::result(ty.basic_void));
         pair.save.as_modifiable().set_result_decl(gw::decl::result(ty.basic_void));
         
         pair.read.nth_parameter(0).make_used();
         pair.save.nth_parameter(0).make_used();
         
         auto ctxt = codegen::instructions::utils::generation_context(this->whole_struct);
         ctxt.state_ptr = codegen::optional_value_pair(
            pair.read.nth_parameter(0).as_value(),
            pair.save.nth_parameter(0).as_value()
         );
         auto expr = instructions_by_sector[i]->generate(ctxt);
         
         if (!expr.read.is<gw::expr::local_block>()) {
            gw::expr::local_block block;
            block.statements().append(expr.read);
            expr.read = block;
         }
         if (!expr.save.is<gw::expr::local_block>()) {
            gw::expr::local_block block;
            block.statements().append(expr.save);
            expr.save = block;
         }
         
         pair.read.set_is_defined_elsewhere(false);
         pair.save.set_is_defined_elsewhere(false);
         {
            auto block_read = expr.read.as<gw::expr::local_block>();
            auto block_save = expr.save.as<gw::expr::local_block>();
            pair.read.as_modifiable().set_root_block(block_read);
            pair.save.as_modifiable().set_root_block(block_save);
         }
         
         // expose these identifiers so we can inspect them with our debug-dump pragmas.
         // (in release builds we'll likely want to remove this, so user code can't call 
         // these functions or otherwise access them directly.)
         pair.read.introduce_to_current_scope();
         pair.save.introduce_to_current_scope();
         
         this->per_sector.push_back(pair);
      }
   }
   
   bool generation_result::_get_or_declare_top_level_functions(const generation_request& request) {
      const auto& gs = basic_global_state::get_fast();
      const auto& ty = gw::builtin_types::get_fast();
      
      auto& pair = this->top_level;
      //
      // Get-or-create the functions; then create their bodies.
      //
      bool can_generate_bodies = true;
      {  // void __lu_bitpack_read_by_sector(const buffer_byte_type* src, int sector_id);
         gw::identifier    id   = gw::identifier(request.function_names.read);
         gw::optional_node node = lookup_name(id.unwrap());
         if (node) {
            auto decl = node->as<gw::decl::function>();
            pair.read = decl;
            if (decl.has_body()) {
               error("cannot generate a definition for function %qE, as it already has a definition", id.unwrap());
               can_generate_bodies = false;
            }
         } else {
            pair.read = gw::decl::function(
               id.name().data(),
               gw::type::function(
                  ty.basic_void,
                  // args:
                  gs.global_options.types.buffer_byte_ptr->remove_pointer().add_const().add_pointer(),
                  ty.basic_int // int sectorID
               )
            );
            inform(UNKNOWN_LOCATION, "generated function %qE did not already have a declaration; implicit declaration generated", id.unwrap());
         }
      }
      {  // void __lu_bitpack_save_by_sector(buffer_byte_type* dst, int sector_id);
         gw::identifier    id   = gw::identifier(request.function_names.save);
         gw::optional_node node = lookup_name(id.unwrap());
         if (node) {
            auto decl = node->as<gw::decl::function>();
            pair.save = decl;
            if (decl.has_body()) {
               error("cannot generate a definition for function %qE, as it already has a definition", id.unwrap());
               can_generate_bodies = false;
            }
         } else {
            pair.save = gw::decl::function(
               id.name().data(),
               gw::type::function(
                  ty.basic_void,
                  // args:
                  *gs.global_options.types.buffer_byte_ptr,
                  ty.basic_int // int sectorID
               )
            );
            inform(UNKNOWN_LOCATION, "generated function %qE did not already have a declaration; implicit declaration generated", id.unwrap());
         }
      }
      return can_generate_bodies;
   }
   
   void generation_result::_generate_top_level_function(const generation_request& request, size_t sector_count, bool is_read) {
      const auto& gs = basic_global_state::get_fast();
      const auto& ty = gw::builtin_types::get_fast();
      
      auto func     = *(is_read ? this->top_level.read : this->top_level.save);
      auto func_mod = func.as_modifiable();
      
      gw::expr::local_block root_block;
      gw::statement_list    statements = root_block.statements();
      
      auto state_decl = gw::decl::variable("__lu_bitstream_state", *gs.global_options.types.bitstream_state);
      state_decl.make_artificial();
      state_decl.make_used();
      //
      statements.append(state_decl.make_declare_expr());
      
      {  // lu_BitstreamInitialize(&state, dst);
         auto src_arg = func.nth_parameter(0).as_value();
         if (is_read) {
            //
            // Cast away const-ness. The bitstream type won't modify the buffer 
            // if we only use "read" calls, but the same type is used for read 
            // and write calls and thus needs non-const access.
            //
            auto pointer_type = src_arg.value_type();
            auto value_type   = pointer_type.remove_pointer();
            if (value_type.is_const()) {
               src_arg = src_arg.conversion_sans_bytecode(value_type.remove_const().add_pointer());
            }
         }
         statements.append(
            gw::expr::call(
               *gs.global_options.functions.stream_state_init,
               // args:
               state_decl.as_value().address_of(), // &state
               src_arg // dst
            )
         );
      }
   
      std::vector<gw::expr::call> calls;
      calls.reserve(sector_count);
      for(size_t i = 0; i < sector_count; ++i) {
         calls.push_back(gw::expr::call(
            is_read ? this->per_sector[i].read : this->per_sector[i].save,
            // args:
            state_decl.as_value().address_of()
         ));
      }
      
      gw::flow::simple_if_else_set branches;
      for(size_t i = 0; i < calls.size(); ++i) {
         branches.add_branch(
            func.nth_parameter(1).as_value().cmp_is_equal(
               gw::constant::integer(ty.basic_int, i)
            ),
            calls[i]
         );
      }
      if (branches.result)
         statements.append(*branches.result);
      
      func.set_is_defined_elsewhere(false);
      func_mod.set_result_decl(gw::decl::result(ty.basic_void));
      func_mod.set_root_block(root_block);
      
      // expose these identifiers so we can inspect them with our debug-dump pragmas.
      // (in release builds we'll likely want to remove this, so user code can't call 
      // these functions or otherwise access them directly.)
      func.introduce_to_current_scope();
   }
   
   bool generation_result::generate(const generation_request& request, const std::vector<std::unique_ptr<instructions::base>>& instructions_by_sector) {
      this->_generate_per_sector_functions(request, instructions_by_sector);
      if (!this->_get_or_declare_top_level_functions(request))
         return false;
      this->_generate_top_level_function(request, instructions_by_sector.size(), true);
      this->_generate_top_level_function(request, instructions_by_sector.size(), false);
      return true;
   }
}