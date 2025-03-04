#include "codegen/optional_value_pair.h"
#include <cassert>
#include "codegen/decl_descriptor.h"
#include "codegen/decl_descriptor_pair.h"
namespace gw {
   using namespace gcc_wrappers;
}

namespace codegen {
   optional_value_pair::optional_value_pair(const decl_descriptor_pair& pair) {
      assert(pair.read != nullptr);
      assert(pair.save != nullptr);
      //
      // Unwrap and re-wrap the DECLs to strip constness. (The containing 
      // `decl_descriptor` is const, and so the embedded DECL wrappers are 
      // const as well. We don't want to be able to rebind them, but we do 
      // want non-const access to the wrapped node. This is ugly.)
      //
      this->read = gw::decl::base_value::wrap((tree) pair.read->decl.unwrap()).as_value();
      this->save = gw::decl::base_value::wrap((tree) pair.save->decl.unwrap()).as_value();
   }
   optional_value_pair::optional_value_pair(gw::value read, gw::value save) : read(read), save(save) {}
   optional_value_pair::optional_value_pair(gw::optional_value read, gw::optional_value save) : read(read), save(save) {}
   
   optional_value_pair optional_value_pair::access_array_element(const optional_value_pair& i) const {
      assert(!!this->read);
      assert(!!this->save);
      optional_value_pair out;
      out.read = gw::value::wrap((tree) this->read.unwrap()).access_array_element(*i.read);
      out.save = gw::value::wrap((tree) this->save.unwrap()).access_array_element(*i.save);
      return out;
   }
   optional_value_pair optional_value_pair::access_member(const char* name) const {
      assert(!!this->read);
      assert(!!this->save);
      optional_value_pair out;
      out.read = gw::value::wrap((tree) this->read.unwrap()).access_member(name);
      out.save = gw::value::wrap((tree) this->save.unwrap()).access_member(name);
      return out;
   }
   optional_value_pair optional_value_pair::dereference() const {
      assert(!!this->read);
      assert(!!this->save);
      optional_value_pair out;
      out.read = gw::value::wrap((tree) this->read.unwrap()).dereference();
      out.save = gw::value::wrap((tree) this->save.unwrap()).dereference();
      return out;
   }
}