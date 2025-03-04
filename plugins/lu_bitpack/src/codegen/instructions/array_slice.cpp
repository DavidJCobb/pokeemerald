#include "codegen/instructions/array_slice.h"
#include "codegen/instructions/utils/generation_context.h"
#include "codegen/decl_dictionary.h"
#include "gcc_wrappers/flow/simple_for_loop.h"
#include "gcc_wrappers/builtin_types.h"
namespace gw {
   using namespace gcc_wrappers;
}

namespace codegen::instructions {
   array_slice::array_slice()
   :
      loop_index({
         .variables = {
            .read = gw::decl::variable(
               "__i_read",
               gw::builtin_types::get().basic_int
            ),
            .save = gw::decl::variable(
               "__i_save",
               gw::builtin_types::get_fast().basic_int
            )
         },
      })
   {
      this->loop_index.variables.read.make_artificial();
      this->loop_index.variables.save.make_artificial();
       
      auto& dict = decl_dictionary::get();
      this->loop_index.descriptors = decl_descriptor_pair{
         .read = &dict.describe(this->loop_index.variables.read),
         .save = &dict.describe(this->loop_index.variables.save),
      };
   }
   
   /*virtual*/ expr_pair array_slice::generate(const utils::generation_context& ctxt) const {
      const auto& ty = gw::builtin_types::get();
      
      //
      // TODO: Investigate optimizing arrays of opaque buffers so that 
      // we memcpy the whole array.
      //
      
      gw::flow::simple_for_loop read_loop(ty.basic_int);
      read_loop.counter_bounds = {
         .start     = (intmax_t)this->array.start,
         .last      = (uintmax_t)(this->array.start + this->array.count - 1),
         .increment = 1,
      };
      
      gw::flow::simple_for_loop save_loop(ty.basic_int);
      save_loop.counter_bounds = read_loop.counter_bounds;
      
      // Overwrite the loop counter that `simple_for_loop` creates, since 
      // we already created one and it's already referenced somewhere.
      read_loop.counter = this->loop_index.variables.read;
      save_loop.counter = this->loop_index.variables.save;
      
      gw::statement_list read_loop_body;
      gw::statement_list save_loop_body;
      
      for(auto& child_ptr : this->instructions) {
         auto pair = child_ptr->generate(ctxt);
         read_loop_body.append(pair.read);
         save_loop_body.append(pair.save);
      }
      
      read_loop.bake(std::move(read_loop_body));
      save_loop.bake(std::move(save_loop_body));
      
      return expr_pair(read_loop.enclosing, save_loop.enclosing);
   }
}