#include "bitpacking/attribute_attempted_on.h"
#include <cassert>
#include "gcc_wrappers/attribute.h"
#include "gcc_wrappers/attribute_list.h"
#include "gcc_wrappers/identifier.h"
namespace gw {
   using namespace gcc_wrappers;
}

namespace bitpacking {
   extern bool attribute_attempted_on(gw::attribute_list list, lu::strings::zview attr_name) {
      for (auto attr : list) {
         const auto name = attr.name();
         if (name == attr_name) {
            return true;
         }
         if (name == "lu bitpack invalid attribute name") {
            auto id = attr.arguments().front().as<gw::identifier>();
            if (id.name() == attr_name)
               return true;
         }
      }
      return false;
   }
}