#include "bitpacking/verify_union_members.h"
#include <cstdint>
#include <optional>
#include <unordered_map>
#include "bitpacking/for_each_influencing_entity.h"
#include "gcc_wrappers/constant/integer.h"
#include "gcc_wrappers/decl/field.h"
#include "gcc_wrappers/attribute.h"
#include "gcc_wrappers/attribute_list.h"
#include <diagnostic.h>
namespace gw {
   using namespace gcc_wrappers;
}

namespace bitpacking {
   extern bool verify_union_members(gw::type::untagged_union type, bool silent) {
      bool valid = true;
      
      std::unordered_map<intmax_t, gw::decl::field> seen_ids;
      type.for_each_field([silent, &valid, &seen_ids](gw::decl::field decl) {
         bool has_default = false;
         bool has_tag_id  = false;
         bool is_omitted  = false;
         std::optional<intmax_t> tag_id;
         //
         // Gather the info to check.
         //
         for(auto attr : decl.attributes()) {
            auto name = attr.name();
            if (name == "lu_bitpack_omit") {
               is_omitted = true;
               continue;
            }
            if (name == "lu_bitpack_default_value") {
               has_default = true;
               continue;
            }
            if (name == "lu_bitpack_union_member_id") {
               has_tag_id = true;
               tag_id     = attr.arguments().front().as<gw::constant::integer>().value<intmax_t>();
               continue;
            }
            if (name == "lu bitpack invalid attribute name") {
               auto node = attr.arguments().front().unwrap();
               assert(node != NULL_TREE && TREE_CODE(node) == IDENTIFIER_NODE);
               name = IDENTIFIER_POINTER(node);
               
               if (name == "lu_bitpack_default_value") {
                  has_default = true;
                  continue;
               }
               if (name == "lu_bitpack_union_member_id") {
                  has_tag_id = true;
                  continue;
               }
            }
         }
         //
         // Fields must either be omitted or have an ID for serialization.
         //
         if (is_omitted) {
            //
            // Fields that are omitted AND have a default value must have an ID.
            //
            if (has_default && !has_tag_id) {
               valid = false;
               if (!silent) {
                  error_at(decl.source_location(), "if a tagged union member is omitted from bitpacking and has a default value via %<lu_bitpack_default_value%>, then it must also have a unique ID via %<lu_bitpack_union_member_id%>");
                  inform(decl.source_location(), "either give this union member a unique ID (to indicate when we should even use the default value) or remove the default value (since it will never be used)");
               }
            }
            return;
         }
         if (!has_tag_id) {
            valid = false;
            if (!silent) {
               error_at(decl.source_location(), "tagged union members must either be omitted from bitpacking or have a unique ID via %<lu_bitpack_union_member_id%>");
            }
            return;
         }
         //
         // IDs for serialization, if present, must be unique.
         //
         if (tag_id.has_value()) {
            bool seen = false;
            for(const auto& pair : seen_ids) {
               if (pair.first == *tag_id) {
                  valid = false;
                  seen  = true;
                  if (!silent) {
                     error_at(decl.source_location(), "tagged union member %<%s%> has the same unique ID as other members of the same tagged union", decl.name().data());
                     inform(
                        pair.second.source_location(),
                        "unique ID %r%" PRIdMAX "%R was first used here",
                        "quote",
                        *tag_id
                     );
                  }
                  break;
               }
            }
            if (!seen) {
               // seen_ids[*tag_id] = decl; // impossible if not default-constructible
               seen_ids.insert(std::pair<intmax_t, gw::decl::field>(*tag_id, decl));
            }
         }
      });
      return valid;
   }
}