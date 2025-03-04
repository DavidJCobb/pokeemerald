#include "codegen/instructions/base.h"
#include "codegen/instructions/utils/generation_context.h"
#include "gcc_wrappers/expr/local_block.h"
namespace gw {
   using namespace gcc_wrappers;
}

namespace codegen::instructions {
   /*virtual*/ expr_pair container::generate(const utils::generation_context& ctxt) const {
      if (this->instructions.size() == 1) {
         return this->instructions.front()->generate(ctxt);
      }
      gw::expr::local_block block_read;
      gw::expr::local_block block_save;
      {
         auto statements_read = block_read.statements();
         auto statements_save = block_save.statements();
         for(auto& child_ptr : this->instructions) {
            auto pair = child_ptr->generate(ctxt);
            statements_read.append(pair.read);
            statements_save.append(pair.save);
         }
      }
      return expr_pair(block_read, block_save);
   }
}