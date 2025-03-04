#include "bitpacking/verify_union_external_tag.h"
#include <string>
#include "gcc_wrappers/type/record.h"
#include "gcc_wrappers/type/untagged_union.h"
#include "gcc_wrappers/attribute.h"
#include "gcc_wrappers/attribute_list.h"
#include <diagnostic.h>
namespace gw {
   using namespace gcc_wrappers;
}

// For comparing bitpacking options across members
#include "bitpacking/data_options.h"

namespace bitpacking {
   static bool _compare_fields(
      gw::decl::field decl_a,
      gw::decl::field decl_b
   ) {
      if (decl_a.name() != decl_b.name())
         return false;
      if (decl_a.value_type() != decl_b.value_type())
         return false;
      
      //
      // TODO: Compare bitpacking options (accounting for any inherited 
      //       from the fields' types).
      //
      data_options options_a;
      data_options options_b;
      options_a.load(decl_a);
      if (options_a.valid()) { // don't compare error'd options
         options_b.load(decl_b);
         if (options_b.valid()) { // don't compare error'd options
            //
            // We intentionally do not check options that do not change 
            // the serialized format, e.g. default values.
            //
            if (!options_a.same_format_as(options_b))
               return false;
         }
      }
      
      return true;
   }
   
   // Return `false` if invalid.
   extern bool verify_union_internal_tag(
      gw::type::untagged_union type,
      lu::strings::zview       tag_identifier,
      bool silent
   ) {
      if (tag_identifier.empty())
         return false;
      
      bool valid = true;
      
      std::vector<gw::decl::field> matched_members;
      {
         bool is_first = true;
         type.for_each_field([silent, &matched_members, &valid, &is_first](gw::decl::field decl) {
            auto decl_name  = decl.name();
            auto value_type = decl.value_type();
            {
               bool omitted = false;
               for(auto attr : decl.attributes()) {
                  auto name = attr.name();
                  if (name == "lu_bitpack_omit") {
                     omitted = true;
                     break;
                  }
                  if (name == "lu bitpack invalid attribute name") {
                     auto node = attr.arguments().front().unwrap();
                     assert(node != NULL_TREE && TREE_CODE(node) == IDENTIFIER_NODE);
                     name = IDENTIFIER_POINTER(node);
                     
                     if (name == "lu_bitpack_omit") {
                        omitted = true;
                        break;
                     }
                  }
               }
               if (omitted) {
                  return;
               }
            }
            if (!value_type.is_record()) {
               if (!silent) {
                  if (decl_name.empty()) {
                     error_at(decl.source_location(), "%<lu_bitpack_union_internal_tag%>: all non-omitted members of an internally tagged union must be struct types; this unnamed member is not");
                  } else {
                     error_at(decl.source_location(), "%<lu_bitpack_union_internal_tag%>: all non-omitted members of an internally tagged union must be struct types; member %<%s%> is not", decl.name().data());
                  }
               }
               is_first = false;
               valid    = false;
               return;
            }
            auto record_type = value_type.as_record();
            if (is_first) {
               record_type.for_each_referenceable_field([&matched_members](gw::decl::field nest) {
                  matched_members.push_back(nest);
               });
               is_first = false;
            } else {
               size_t i = 0;
               record_type.for_each_referenceable_field([&matched_members, &i](gw::decl::field field) {
                  if (i >= matched_members.size()) {
                     return false;
                  }
                  auto a  = field;
                  auto b  = matched_members[i];
                  if (!_compare_fields(a, b)) {
                     //matched_members.resize(i); // not default-constructible
                     matched_members.erase(matched_members.begin() + i, matched_members.end());
                     return false;
                  }
                  
                  ++i;
                  return true;
               });
            }
         });
         if (is_first) {
            //
            // We never saw a usable member.
            //
            if (!silent) {
               auto name = type.name();
               if (name.empty()) {
                  error_at(type.source_location(), "%<lu_bitpack_union_internal_tag%>: the union does not have any non-omitted struct-type members and so cannot contain an internal tag");
               } else {
                  error_at(type.source_location(), "%<lu_bitpack_union_internal_tag%>: union type %<%s%> does not have any non-omitted struct-type members and so cannot contain an internal tag", name.data());
               }
            }
            return false;
         }
      }
      
      if (matched_members.empty()) {
         if (!silent) {
            auto name = type.name();
            if (name.empty()) {
               error_at(type.source_location(), "%<lu_bitpack_union_internal_tag%>: the union%'s non-omitted members must all be struct types, those structs%' first field(s) must be identical, and the union%'s tag must be one of those fields. none of the structs have any first fields in common");
            } else {
               error_at(type.source_location(), "%<lu_bitpack_union_internal_tag%>: the non-omitted members of union type %<%s%> must all be struct types, those structs%' first field(s) must be identical, and the union%'s tag must be one of those fields. none of the structs have any first fields in common", name.data());
            }
         }
         return false;
      }
      bool found = false;
      for(auto decl : matched_members) {
         if (decl.name() == tag_identifier) {
            found = true;
            
            auto decl_type = decl.value_type();
            if (!decl_type.is_integral()) {
               error_at(type.source_location(), "%<lu_bitpack_union_internal_tag%>: union type %<%s%> cannot use member-of-member %<%s%> as its tag, because that member is not of an integral type", type.name().data(), tag_identifier.c_str());
               valid = false;
            }
            
            break;
         }
      }
      if (!found) {
         if (!silent) {
            auto name = type.name();
            if (name.empty()) {
               error_at(type.source_location(), "%<lu_bitpack_union_internal_tag%>: the union%'s non-omitted members must all be struct types, those structs%' first field(s) must be identical, and the union%'s tag must be one of those fields. tag identifier %<%s%> does not refer to any of those fields", tag_identifier.c_str());
            } else {
               error_at(type.source_location(), "%<lu_bitpack_union_internal_tag%>: the non-omitted members of union type %<%s%> must all be struct types, those structs%' first field(s) must be identical, and the union%'s tag must be one of those fields. tag identifier %<%s%> does not refer to any of those fields", name.data(), tag_identifier.c_str());
            }
            if (matched_members.size() == 1) {
               inform(type.source_location(), "the only common field between those structs is %<%s%>", matched_members[0].name().data());
            } else {
               inform(
                  type.source_location(),
                  "those structs have their first %u fields (from %<%s%> through %<%s%>) in common",
                  (int)matched_members.size(),
                  matched_members.front().name().data(),
                  matched_members.back().name().data()
               );
            }
         }
         return false;
      }
      
      return valid;
   }
}