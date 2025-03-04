#include "codegen/instructions/union_switch.h"
#include <cassert>
#include "codegen/instructions/utils/generation_context.h"
#include "gcc_wrappers/constant/integer.h"
#include "gcc_wrappers/expr/local_block.h"
#include "gcc_wrappers/expr/ternary.h"
#include "gcc_wrappers/flow/simple_if_else_set.h"
#include "gcc_wrappers/builtin_types.h"
namespace gw {
   using namespace gcc_wrappers;
}

namespace codegen::instructions {
   /*virtual*/ expr_pair union_switch::generate(const utils::generation_context& ctxt) const {
      if (this->cases.empty()) {
         return expr_pair(
            gw::expr::base::wrap(build_empty_stmt(UNKNOWN_LOCATION)),
            gw::expr::base::wrap(build_empty_stmt(UNKNOWN_LOCATION))
         );
      }
      
      auto operand      = this->condition_operand.as_value_pair();
      auto operand_type = operand.read->value_type();
      assert(operand_type.is_boolean() || operand_type.is_enum() || operand_type.is_integer());
      
      gw::flow::simple_if_else_set branches_read;
      gw::flow::simple_if_else_set branches_save;
      for(auto& pair : this->cases) {
         auto pair_rhs  = gw::constant::integer(operand_type.as_integral(), pair.first);
         auto pair_gene = pair.second->generate(ctxt);
         
         branches_read.add_branch(
            operand.read->cmp_is_equal(pair_rhs),
            pair_gene.read
         );
         branches_save.add_branch(
            operand.save->cmp_is_equal(pair_rhs),
            pair_gene.save
         );
      }
      if (this->else_case.get()) {
         auto pair_gene = this->else_case->generate(ctxt);
         if (this->cases.empty()) {
            return pair_gene;
         }
         branches_read.set_else_branch(pair_gene.read);
         branches_save.set_else_branch(pair_gene.save);
      }
      return expr_pair(*branches_read.result, *branches_save.result);
   }
}