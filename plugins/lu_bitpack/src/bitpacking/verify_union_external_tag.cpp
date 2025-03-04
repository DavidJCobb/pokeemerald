#include "bitpacking/verify_union_external_tag.h"
#include <string>
#include "gcc_wrappers/type/untagged_union.h"
#include "gcc_wrappers/scope.h"
#include <diagnostic.h>
namespace gw {
   using namespace gcc_wrappers;
}

namespace bitpacking {
   // Return `false` if invalid.
   extern bool verify_union_external_tag(
      gw::decl::field    union_decl,
      lu::strings::zview tag_identifier,
      bool silent
   ) {
      if (tag_identifier.empty())
         return false;
      //
      // Requirement: The union-type field must be a member of a struct.
      //
      auto scope = union_decl.context();
      if (!scope)
         return true;
      assert(scope->is_type());
      auto type = scope->as_type().as_container();
      if (!type.is_record()) {
         if (!silent) {
            error_at(union_decl.source_location(), "%<lu_bitpack_union_external_tag%>: an externally tagged union must be a member of a struct");
            auto pp = type.pretty_print();
            inform(type.source_location(), "union-type data member %<%s%> is a member of non-struct type %<%s%>", union_decl.name().data(), pp.c_str());
         }
         return false;
      }
      //
      // Requirement: the tag identifier specified by the union-type field must be 
      // a member of the immediate containing struct, and must precede the union.
      //
      gw::decl::optional_field tag;
      bool tag_after = false;
      type.for_each_field([&union_decl, tag_identifier, &tag_after, &tag](gw::decl::field decl) {
         if (decl == union_decl) {
            if (tag.empty())
               tag_after = true;
            return true;
         }
         if (decl.name() == tag_identifier) {
            tag = decl;
            return false;
         }
         return true;
      });
      if (!tag) {
         if (!silent) {
            error_at(union_decl.source_location(), "%<lu_bitpack_union_external_tag%>: union-type data member %<%s%> specifies a non-existent sibling member as its tag", union_decl.name().data());
            
            if (type.name().empty()) {
               inform(type.source_location(), "containing struct does not have a field named %<%s%>", tag_identifier.c_str());
            } else {
               auto pp = type.pretty_print();
               inform(type.source_location(), "containing struct %<%s%> does not have a field named %<%s%>", pp.c_str(), tag_identifier.c_str());
            }
         }
         return false;
      }
      bool valid = true;
      {
         auto decl_type = tag->value_type();
         if (!decl_type.is_integral()) {
            if (!silent) {
               error_at(union_decl.source_location(), "%<lu_bitpack_union_external_tag%>: union-type data member %<%s%> cannot use member %<%s%> as its tag, because that member is not of an integral type", union_decl.name().data(), tag_identifier.c_str());
            }
            valid = false;
         }
      }
      if (tag_after) {
         if (!silent) {
            error_at(union_decl.source_location(), "%<lu_bitpack_union_external_tag%>: union-type data member %<%s%> appears before its sibling tag member %<%s%>", union_decl.name().data(), tag_identifier.c_str());
            inform(tag->source_location(), "the tag is defined here");
         }
         valid = false;
      }
      return valid;
   }
}