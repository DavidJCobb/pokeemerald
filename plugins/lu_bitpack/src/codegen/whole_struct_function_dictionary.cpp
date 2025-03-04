#include "codegen/whole_struct_function_dictionary.h"
#include <cassert>
namespace gw {
   using namespace gcc_wrappers;
}

namespace codegen {
   optional_func_pair whole_struct_function_dictionary::get_functions_for(gw::type::base type) const{
      auto it = this->_functions.find(type);
      if (it == this->_functions.end())
         return {};
      return it->second.functions;
   }
   const instructions::base* whole_struct_function_dictionary::get_instructions_for(gcc_wrappers::type::base type) const {
      auto it = this->_functions.find(type);
      if (it == this->_functions.end())
         return nullptr;
      return it->second.instructions_root.get();
   }
   
   void whole_struct_function_dictionary::add_functions_for(gw::type::base type, whole_struct_function_info&& info) {
      auto& slot = this->_functions[type];
      assert(!slot.functions.read && !slot.functions.save);
      slot = std::move(info);
   }
}