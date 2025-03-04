#include "codegen/expr_pair.h"

namespace codegen {
   expr_pair::expr_pair(const optional_expr_pair& pair)
   :
      read(*pair.read),
      save(*pair.save)
   {}
   
   expr_pair::expr_pair(gcc_wrappers::expr::base read, gcc_wrappers::expr::base save)
   :
      read(read),
      save(save)
   {}
}