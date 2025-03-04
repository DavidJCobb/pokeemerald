#include "codegen/func_pair.h"
namespace gw {
   using namespace gcc_wrappers;
}

namespace codegen {
   optional_func_pair& optional_func_pair::operator=(const func_pair& src) noexcept {
      this->read = src.read;
      this->save = src.save;
      return *this;
   }
   
   //
   
   func_pair::func_pair(const optional_func_pair& pair)
   :
      read(*(gw::decl::optional_function)pair.read),
      save(*(gw::decl::optional_function)pair.save)
   {}
   
   func_pair::func_pair(gw::decl::function read, gw::decl::function save)
   :
      read(read),
      save(save)
   {}
}