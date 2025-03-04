#include <cassert>
#include "codegen/value_path.h"
#include "gcc_wrappers/constant/integer.h"
#include "gcc_wrappers/decl/param.h"
#include "gcc_wrappers/decl/variable.h"
#include "gcc_wrappers/builtin_types.h"
#include "gcc_wrappers/value.h"
namespace gw {
   using namespace gcc_wrappers;
}

namespace codegen {
   void value_path::access_array_element(decl_descriptor_pair pair) {
      this->segments.emplace_back().data.emplace<1>() = pair;
   }
   void value_path::access_array_element(size_t n) {
      this->segments.emplace_back().data.emplace<size_t>() = n;
   }
   void value_path::access_member(const decl_descriptor& desc) {
      this->segments.emplace_back().data.emplace<0>() = decl_descriptor_pair{
         .read = &desc,
         .save = &desc
      };
   }
   
   void value_path::replace(decl_descriptor_pair pair) {
      this->segments.clear();
      this->segments.emplace_back().data.emplace<0>() = pair;
   }
   
   optional_value_pair value_path::as_value_pair() const {
      const auto& ty = gw::builtin_types::get();
      
      optional_value_pair out;
      
      bool first = true;
      for(auto& segm : this->segments) {
         if (first) {
            assert(!segm.is_array_access());
            auto& pair = segm.member_descriptor();
            assert(pair.read != nullptr);
            assert(pair.save != nullptr);
            auto decl_r = pair.read->decl;
            auto decl_s = pair.save->decl;
            assert(decl_r.code() == decl_s.code());
            assert(decl_r.is<gw::decl::param>() || decl_r.is<gw::decl::variable>());
            out = optional_value_pair(pair);
            first = false;
            {
               auto count = pair.read->variable.dereference_count;
               for(size_t i = 0; i < count; ++i) {
                  assert(out.read->value_type().is_pointer());
                  out = out.dereference();
               }
            }
            continue;
         }
         
         if (segm.is_array_access()) {
            optional_value_pair array_index;
            if (std::holds_alternative<size_t>(segm.data)) {
               auto n = std::get<size_t>(segm.data);
               array_index.read = gw::constant::integer(ty.basic_int, n);
               array_index.save = gw::constant::integer(ty.basic_int, n);
            } else {
               array_index = optional_value_pair(segm.array_loop_counter_descriptor());
            }
            out = out.access_array_element(array_index);
         } else {
            auto& pair = segm.member_descriptor();
            assert(pair.read != nullptr);
            assert(pair.save != nullptr);
            assert(pair.read->variable.dereference_count == 0);
            out = out.access_member(pair.read->decl.name().data());
         }
      }
      
      return out;
   }
   const bitpacking::data_options& value_path::bitpacking_options() const {
      const decl_descriptor* desc = nullptr;
      for(auto& segm : this->segments) {
         if (segm.is_array_access())
            continue;
         desc = segm.member_descriptor().read;
      }
      return desc->options;
   }
}