#include <inttypes.h> // PRIdMAX
#include "lu/stringf.h"
#include "codegen/serialization_items/condition.h"

namespace codegen::serialization_items {
   std::string condition_type::to_string() const {
      std::string out;
      
      bool first = true;
      for(auto& segm : this->lhs) {
         if (!first)
            out += '.';
         first = false;
         
         out += segm.to_string();
      }
      if (this->is_else) {
         out += " else";
      } else {
         out += " == ";
         out += lu::stringf("%" PRIdMAX, this->rhs);
      }
      
      return out;
   }
}