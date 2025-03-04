#include "attribute_handlers/bitpack_union_internal_tag.h"
#include "attribute_handlers/helpers/bp_attr_context.h"
#include "attribute_handlers/generic_bitpacking_data_option.h"
#include "gcc_wrappers/constant/string.h"
#include "gcc_wrappers/decl/base_value.h"
#include "gcc_wrappers/type/array.h"
#include "gcc_wrappers/type/base.h"
#include "gcc_wrappers/type/untagged_union.h"
#include "gcc_wrappers/identifier.h"
#include <diagnostic.h>
#include "bitpacking/verify_union_internal_tag.h"
#include "bitpacking/verify_union_members.h"
namespace gw {
   using namespace gcc_wrappers;
}

namespace attribute_handlers {
   extern tree bitpack_union_internal_tag(tree* node_ptr, tree name, tree args, int flags, bool* no_add_attrs) {
      auto result = generic_bitpacking_data_option(node_ptr, name, args, flags, no_add_attrs);
      if (*no_add_attrs) {
         return result;
      }
      
      if (flags & ATTR_FLAG_INTERNAL) {
         return NULL_TREE;
      }
      helpers::bp_attr_context context(node_ptr, name, flags);
      
      gw::type::optional_untagged_union union_type;
      {
         auto type = context.type_of_target();
         while (type.is_array())
            type = type.as_array().value_type();
         if (type.is_union())
            union_type = type.as_union();
      }
      if (!union_type) {
         context.report_error("can only be applied to unions");
      }
      
      gw::constant::optional_string data;
      {
         auto next = TREE_VALUE(args);
         if (next != NULL_TREE && gw::constant::string::raw_node_is(next)) {
            data = next;
         } else {
            context.report_error("argument must be a string constant");
         }
      }
      
      if (data) {
         auto view = data->value();
         if (view.empty()) {
            context.report_error("tag identifier cannot be blank");
         }
         if (gw::decl::base_value::raw_node_is(node_ptr[0])) {
            if (union_type && union_type->is_complete()) {
               //
               // There are three cases we need to handle:
               //
               //  - The attribute is applied to a variable or field declaration.
               // 
               //  - The variable is applied to a typedef (TYPE_DECL), i.e. the 
               //    author used `[attr] typedef union { ... } foo`.
               //
               //  - The variable is applied to a type, i.e. the author wrote 
               //    `union [attr] NameIfAnyHere { ... }`.
               //
               // Types can only be validated when GCC finishes parsing them; 
               // attributes on types run their handlers before the type is 
               // complete and so can't tell what's in the union. Fortunately, 
               // plug-ins can register a handler to run when types are finished, 
               // so we'll just run these validation steps (and some others) at 
               // that time.
               //
               // We can still validate attributes applied to variables or fields 
               // from within the attribute handler, since the union type has to 
               // be complete at that time (or the C code will be invalid).
               //
               // Note that we can't do this kind of validation for externally 
               // tagged unions' attribute handlers, because the union-type field 
               // would exist inside of a type that's still being parsed (and as 
               // such has an unavailable/incomplete member list).
               //
               bool success =
                  bitpacking::verify_union_members(*union_type) &&
                  bitpacking::verify_union_internal_tag(*union_type, view.data(), false)
               ;
               if (!success)
                  context.report_error("encountered errors (see previous message(s))");
            }
         }
      }
      
      *no_add_attrs = true;
      if (!context.has_any_errors()) {
         assert(!!data);
         auto view = data->value();
         
         gw::list_node args({}, gw::identifier(view.data()));
         context.reapply_with_new_args(args);
      }
      return NULL_TREE;
   }
}