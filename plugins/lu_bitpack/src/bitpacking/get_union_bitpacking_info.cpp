#include "bitpacking/get_union_bitpacking_info.h"
#include "bitpacking/for_each_influencing_entity.h"
#include "gcc_wrappers/type/array.h"
#include "gcc_wrappers/attribute.h"
#include "gcc_wrappers/identifier.h"
namespace gw {
   using namespace gcc_wrappers;
}

namespace bitpacking {
   static void _check_attributes(
      gw::node               node,
      union_bitpacking_info& info,
      gw::attribute_list     list
   ) {
      bool node_is_decl = node.is<gw::decl::base>();
      for(auto attr : list) {
         auto name = attr.name();
         if (name == "lu_bitpack_union_external_tag") {
            auto data = attr.arguments().front().as<gw::identifier>();
            
            auto& opt = info.external;
            auto& dst = opt.has_value() ? *opt : opt.emplace();
            if (!dst.specifying_node || node_is_decl) {
               dst.identifier      = data.name();
               dst.specifying_node = node;
            }
            continue;
         }
         if (name == "lu_bitpack_union_internal_tag") {
            auto data = attr.arguments().front().as<gw::identifier>();
            
            auto& opt = info.internal;
            auto& dst = opt.has_value() ? *opt : opt.emplace();
            if (!dst.specifying_node || node_is_decl) {
               dst.identifier      = data.name();
               dst.specifying_node = node;
            }
            continue;
         }
         if (name == "lu bitpack invalid attribute name") {
            auto data = attr.arguments().front().as<gw::identifier>();
            name = data.name();
            if (name == "lu_bitpack_union_external_tag") {
               auto& opt = info.external;
               auto& dst = opt.has_value() ? *opt : opt.emplace();
               if (!dst.specifying_node || node_is_decl) {
                  dst.specifying_node = node;
               }
               continue;
            }
            if (name == "lu_bitpack_union_internal_tag") {
               auto& opt = info.internal;
               auto& dst = opt.has_value() ? *opt : opt.emplace();
               if (!dst.specifying_node || node_is_decl) {
                  dst.specifying_node = node;
               }
               continue;
            }
         }
      }
   }
   
   extern union_bitpacking_info get_union_bitpacking_info(
      gw::decl::optional_field decl,
      gw::type::optional_base  type
   ) {
      union_bitpacking_info out;
      
      if (decl) {
         _check_attributes(*decl, out, decl->attributes());
         if (!type) {
            type = decl->value_type();
         }
      }
      if (type) {
         bitpacking::for_each_influencing_entity(*type, [&out](gw::node node) {
            gw::attribute_list list;
            if (node.is<gw::type::base>()) {
               list = node.as<gw::type::base>().attributes();
            } else {
               list = node.as<gw::decl::base>().attributes();
            }
            _check_attributes(node, out, list);
         });
      }
      
      return out;
   }
}