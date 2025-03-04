#include "bitpacking/mark_for_invalid_attributes.h"
#include "gcc_wrappers/attribute.h"
#include <stringpool.h> // dependency for <attribs.h>
#include <attribs.h> // decl_attributes
namespace gw {
   using namespace gcc_wrappers;
}
namespace {
   constexpr const char* const sentinel_name = "lu bitpack invalid attributes";
}

namespace bitpacking {
   extern void mark_for_invalid_attributes(gcc_wrappers::decl::base decl) {
      if (decl.attributes().has_attribute(sentinel_name))
         return;
      
      tree nodes[3] = { 0 };
      nodes[0] = decl.unwrap();
      
      decl_attributes(
         nodes,
         gw::attribute(sentinel_name, {}).unwrap(),
         ATTR_FLAG_INTERNAL,
         NULL_TREE
      );
   }
}