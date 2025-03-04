#include "bitpacking/verify_bitpack_attributes_on_type_finished.h"
#include "bitpacking/attribute_attempted_on.h"
#include "bitpacking/for_each_influencing_entity.h"
#include "bitpacking/mark_for_invalid_attributes.h"
#include "lu/strings/zview.h"
#include "gcc_wrappers/decl/base.h"
#include "gcc_wrappers/decl/field.h"
#include "gcc_wrappers/type/container.h"
#include "gcc_wrappers/attribute.h"
#include "gcc_wrappers/attribute_list.h"
#include "bitpacking/get_union_bitpacking_info.h"
#include "bitpacking/verify_union_external_tag.h"
#include "bitpacking/verify_union_internal_tag.h"
#include "bitpacking/verify_union_members.h"
#include <c-family/c-common.h>
#include <diagnostic.h>
namespace gw {
   using namespace gcc_wrappers;
}

static bool _attr_present_or_attempted(gw::decl::field decl, lu::strings::zview attr_name) {
   bool present = false;
   bitpacking::for_each_influencing_entity(decl, [attr_name, &present](gw::node node) -> bool {
      gw::attribute_list list;
      if (node.is<gw::type::base>()) {
         list = node.as<gw::type::base>().attributes();
      } else {
         list = node.as<gw::decl::base>().attributes();
      }
      if (bitpacking::attribute_attempted_on(list, attr_name)) {
         present = true;
         return false;
      }
      return true;
   });
   return present;
}

// Requirements for general container members:
static void _disallow_transforming_non_addressable(gw::decl::field decl) {
   //
   // Non-addressable fields shouldn't be allowed to use transform options. 
   // We can only check this on the type, because as of GCC 11.4.0, the 
   // "decl finished" plug-in callback fires before attributes are attached; 
   // we have to react to the containing type being finished.
   //
   if (_attr_present_or_attempted(decl, "lu_bitpack_transforms")) {
      error_at(
         decl.source_location(),
         "attribute %<lu_bitpack_transform%> (applied to field %<%s%>) can only be applied to objects that support having their addresses taken (e.g. not bit-fields)",
         decl.name().data()
      );
      bitpacking::mark_for_invalid_attributes(decl);
   }
}
static void _disallow_contradictory_union_tagging(
   gw::decl::field decl,
   const bitpacking::union_bitpacking_info& info
) {
   if (!info.external.has_value() || !info.internal.has_value())
      return;
   
   error_at(decl.source_location(), "attributes %<lu_bitpack_union_external_tag%> and %<lu_bitpack_union_internal_tag%> are mutually exclusive but both affect this object");
   bitpacking::mark_for_invalid_attributes(decl);
   
   location_t loc_ext = UNKNOWN_LOCATION;
   location_t loc_int = UNKNOWN_LOCATION;
   if (auto opt = (*info.external).specifying_node; opt) {
      auto node = *opt;
      if (node.is<gw::type::base>())
         loc_ext = node.as<gw::type::base>().source_location();
      else if (node.is<gw::decl::base>())
         loc_ext = node.as<gw::decl::base>().source_location();
   }
   if (auto opt = (*info.internal).specifying_node; opt) {
      auto node = *opt;
      if (node.is<gw::type::base>())
         loc_int = node.as<gw::type::base>().source_location();
      else if (node.is<gw::decl::base>())
         loc_int = node.as<gw::decl::base>().source_location();
   }
   if (loc_ext != loc_int) {
      if (loc_ext != UNKNOWN_LOCATION) {
         inform(loc_ext, "the object is externally tagged as a result of attributes applied here");
      }
      if (loc_int != UNKNOWN_LOCATION) {
         inform(loc_int, "the object is internally tagged as a result of attributes applied here");
      }
   } else if (loc_ext != UNKNOWN_LOCATION) {
      if (loc_int != UNKNOWN_LOCATION) {
         inform(loc_int, "the contradictory tags appeared here");
      }
   }
}

static void _disallow_union_attributes_on_non_union(gw::decl::field decl) {
   auto decl_type = decl.value_type();
   while (decl_type.is_array())
      decl_type = decl_type.as_array().value_type();
   if (!decl_type.is_union()) {
      error_at(
         decl.source_location(),
         "attributes %<lu_bitpack_union_external_tag%> and %<lu_bitpack_union_internal_tag%> can only be applied to unions or arrays of unions"
      );
      bitpacking::mark_for_invalid_attributes(decl);
   }
}

// Requirements for union members:
static void _disallow_union_member_attributes_on_non_union_member(gw::decl::field decl) {
   //
   // The `lu_bitpack_union_member_id` attribute is only permitted on FIELD_DECLs 
   // of a UNION_TYPE. We can only check this when the type is finished, because 
   // DECL_CONTEXT isn't set on a FIELD_DECL at the time that its attributes are 
   // being processed and applied.
   //
   if (bitpacking::attribute_attempted_on(decl.attributes(), "lu_bitpack_union_member_id")) {
      error_at(
         decl.source_location(),
         "attribute %<lu_bitpack_union_member_id%> (applied to field %<%s%>) can only be applied to data members in a union type",
         decl.name().data()
      );
      bitpacking::mark_for_invalid_attributes(decl);
   }
}

namespace bitpacking {
   extern void verify_bitpack_attributes_on_type_finished(gw::type::base node) {
      if (!node.is_container())
         return;
      auto type     = node.as_container();
      bool is_union = node.is_union();
      if (is_union) {
         //
         // In general, we prefer to do error checking and reporting within our 
         // attribute handlers, so that the codegen machinery only ever has to 
         // work with well-formed data. Unfortunately, however, struct and union 
         // types aren't complete at the time their attributes are processed, 
         // so we have to check them for correctness here.
         //
         // Worse yet, this handler runs after attributes have been finalized 
         // for a type and can no longer be modified, so we can't apply error 
         // sentinels to the types from here. The codegen machinery has to run 
         // correctness checks on unions.
         //
         // This is still the best place to actually *report* errors, however, 
         // so we'll run extra checks here in order to do that.
         //
         auto info = get_union_bitpacking_info({}, type);
         if (!info.empty()) {
            verify_union_members(type.as_union());
            if (info.internal.has_value()) {
               verify_union_internal_tag(type.as_union(), (*info.internal).identifier);
            }
         }
         //
         // Validate this union's members:
         //
         //  - All of them need to be given a unique member ID, or be omitted.
         //
         //  - None of them can be externally tagged unions.
         //
         //  - Enforce that transformed members be addressable (i.e. they can't 
         //    be bitfields).
         //
         type.for_each_referenceable_field([](gw::decl::field decl) {
            if (decl.is_non_addressable()) {
               _disallow_transforming_non_addressable(decl);
            }
            
            auto info = get_union_bitpacking_info(decl, {});
            if (!info.empty()) {
               _disallow_union_attributes_on_non_union(decl);
               _disallow_contradictory_union_tagging(decl, info);
               
               // Union members can't be externally tagged unions: there's no 
               // place where the tag could be.
               if (info.external.has_value()) {
                  error_at(
                     decl.source_location(),
                     "attribute %<lu_bitpack_union_external_tag%> (applied to field %<%s%>) can only be applied to union-type data members in a struct type; this is not a member of a struct",
                     decl.name().data()
                  );
                  bitpacking::mark_for_invalid_attributes(decl);
               }
            }
         });
      } else {
         //
         // If this is a struct, then check whether any of its members are of 
         // union types. If so, verify any union-type fields that are externally 
         // tagged. (Fortunately, we *can* mark *those* as invalid because we're 
         // marking the FIELD_DECLs, not their UNION_TYPEs.)
         //
         // While we're at it, we'll also enforce that all transformed members 
         // are addressable (i.e. not bitfields), since that needs checking as 
         // well.
         //
         type.for_each_referenceable_field([](gw::decl::field decl) {
            if (decl.is_non_addressable()) {
               _disallow_transforming_non_addressable(decl);
            }
            
            auto info = get_union_bitpacking_info(decl, {});
            if (!info.empty()) {
               _disallow_union_attributes_on_non_union(decl);
               _disallow_contradictory_union_tagging(decl, info);
               
               // If this union is externally tagged, validate that the tag is 
               // actually present.
               if (info.external.has_value()) {
                  if (!verify_union_external_tag(decl, (*info.external).identifier)) {
                     bitpacking::mark_for_invalid_attributes(decl);
                  }
               }
            }
            
            // You can't give union member IDs to members of a non-union.
            _disallow_union_member_attributes_on_non_union_member(decl);
         });
      }
   }
}