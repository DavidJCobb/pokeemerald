#include "basic_global_state.h"

#include "gcc_wrappers/decl/base.h"
#include "gcc_wrappers/expr/base.h"
#include "gcc_wrappers/type/base.h"
#include "gcc_wrappers/identifier.h"
namespace gw {
   using namespace gcc_wrappers;
}

// For looking up memcpy and memset:
#include <c-family/c-common.h> // lookup_name

#include <diagnostic.h>

static location_t _location_of(tree node) {
   if (DECL_P(node)) {
      return gw::decl::base::wrap(node).source_location();
   }
   if (EXPR_P(node)) {
      return gw::expr::base::wrap(node).source_location();
   }
   return UNKNOWN_LOCATION;
}

void basic_global_state::on_attribute_missed(tree node) {
   this->any_attributes_missed = true;
   if (node == NULL_TREE)
      return;
   auto& dst = this->error_locations.first_missed_attribute;
   if (dst != UNKNOWN_LOCATION)
      return;
   dst = _location_of(node);
}
void basic_global_state::on_attribute_seen(tree applied_to) {
   this->any_attributes_seen = true;
   if (!this->enabled) {
      this->on_attribute_missed(applied_to);
      return;
   }
   if (!this->global_options_seen) {
      auto& flag = this->errors_reported.data_options_before_global_options;
      if (!flag) {
         flag = true;
         error_at(_location_of(applied_to), "bitpacking: data options seen before global options; you need to use %<#pragma lu_bitpack set_options%> first so we can validate attributes against them");
      }
   }
}
void basic_global_state::pull_builtin_functions() {
   {
      auto& decl = this->builtin_functions.memcpy;
      if (!decl) {
         auto id   = gw::identifier("memcpy");
         auto bare = lookup_name(id.unwrap());
         if (bare == NULL_TREE) {
            id   = gw::identifier("__builtin_memcpy");
            bare = lookup_name(id.unwrap());
         }
         decl = bare;
      }
   }
   {
      auto& decl = this->builtin_functions.memset;
      if (!decl) {
         auto id   = gw::identifier("memset");
         auto bare = lookup_name(id.unwrap());
         if (bare == NULL_TREE) {
            id   = gw::identifier("__builtin_memset");
            bare = lookup_name(id.unwrap());
         }
         decl = bare;
      }
   }
}