#include <iostream>
#include <stdexcept>

// Must include gcc-plugin before any other GCC headers.
#include <gcc-plugin.h>
#include <plugin-version.h>

// Required: assert that we're licensed under GPL, or GCC will refuse to run us.
int plugin_is_GPL_compatible;

#include <tree.h>
#include <c-family/c-pragma.h>

// not allowed to be const
static plugin_info _my_plugin_info = {
   .version = "(unversioned)",
   .help    = "there is no help",
};

#include "attribute_handlers/bitpack_as_opaque_buffer.h"
#include "attribute_handlers/bitpack_bitcount.h"
#include "attribute_handlers/bitpack_default_value.h"
#include "attribute_handlers/bitpack_range.h"
#include "attribute_handlers/bitpack_stat_category.h"
#include "attribute_handlers/bitpack_string.h"
#include "attribute_handlers/bitpack_tagged_id.h"
#include "attribute_handlers/bitpack_transforms.h"
#include "attribute_handlers/bitpack_union_external_tag.h"
#include "attribute_handlers/bitpack_union_internal_tag.h"
#include "attribute_handlers/generic_bitpacking_data_option.h"
#include "attribute_handlers/generic_type_or_decl.h"
#include "attribute_handlers/no_op.h"
#include "attribute_handlers/test.h"

namespace _attributes {
   static struct attribute_spec test_attribute = {
      
      // attribute name sans leading or trailing underscores; or NULL to mark 
      // the end of a table of attributes.
      .name = "lu_test_attribute",
      
      .min_length =  0, // min argcount
      .max_length = -1, // max argcount (use -1 for no max)
      .decl_required = false, // if true, applying the attr to a type means it's ignored and warns
      .type_required = false,
      .function_type_required = false,
      .affects_type_identity  = false,
      .handler    = &attribute_handlers::test,
      
      // Can be an array of `attribute_spec::exclusions` objects, which describe 
      // attributes that this attribute is mutually exclusive with. (How do you 
      // mark the end of the array?)
      .exclude = NULL
   };
   
   static struct attribute_spec nonstring = {
      .name = "lu_nonstring",
      .min_length = 0, // min argcount
      .max_length = 0, // max argcount
      .decl_required = false,
      .type_required = false,
      .function_type_required = false,
      .affects_type_identity  = true,
      .handler = &attribute_handlers::generic_type_or_decl,
      .exclude = NULL
   };
   
   // internal use
   static struct attribute_spec internal_invalid = {
      .name = "lu bitpack invalid attributes",
      .min_length = 0, // min argcount
      .max_length = 0, // max argcount
      .decl_required = false,
      .type_required = false,
      .function_type_required = false,
      .affects_type_identity  = true,
      .handler = &attribute_handlers::no_op,
      .exclude = NULL
   };
   static struct attribute_spec internal_invalid_by_name = {
      .name = "lu bitpack invalid attribute name",
      .min_length = 1, // min argcount
      .max_length = 1, // max argcount
      .decl_required = false,
      .type_required = false,
      .function_type_required = false,
      .affects_type_identity  = true,
      .handler = &attribute_handlers::no_op,
      .exclude = NULL
   };
   
   static struct attribute_spec bitpack_as_opaque_buffer = {
      .name = "lu_bitpack_as_opaque_buffer",
      .min_length = 0, // min argcount
      .max_length = 0, // max argcount
      .decl_required = false,
      .type_required = false,
      .function_type_required = false,
      .affects_type_identity  = true,
      .handler = &attribute_handlers::bitpack_as_opaque_buffer,
      .exclude = NULL
   };
   static struct attribute_spec bitpack_bitcount = {
      .name = "lu_bitpack_bitcount",
      .min_length = 1, // min argcount
      .max_length = 1, // max argcount
      .decl_required = false,
      .type_required = false,
      .function_type_required = false,
      .affects_type_identity  = true,
      .handler = &attribute_handlers::bitpack_bitcount,
      .exclude = NULL
   };
   static struct attribute_spec bitpack_default_value = {
      .name = "lu_bitpack_default_value",
      .min_length = 1, // min argcount
      .max_length = 1, // max argcount
      .decl_required = false,
      .type_required = false,
      .function_type_required = false,
      .affects_type_identity  = true,
      .handler = &attribute_handlers::bitpack_default_value,
      .exclude = NULL
   };
   static struct attribute_spec bitpack_omit = {
      .name = "lu_bitpack_omit",
      .min_length = 0, // min argcount
      .max_length = 0, // max argcount
      .decl_required = false,
      .type_required = false,
      .function_type_required = false,
      .affects_type_identity  = true,
      .handler = &attribute_handlers::generic_bitpacking_data_option,
      .exclude = NULL
   };
   static struct attribute_spec bitpack_range = {
      .name = "lu_bitpack_range",
      .min_length = 2, // min argcount
      .max_length = 2, // max argcount
      .decl_required = false,
      .type_required = false,
      .function_type_required = false,
      .affects_type_identity  = true,
      .handler = &attribute_handlers::bitpack_range,
      .exclude = NULL
   };
   static struct attribute_spec bitpack_stat_category = {
      .name = "lu_bitpack_stat_category",
      .min_length = 1, // min argcount
      .max_length = 1, // max argcount
      .decl_required = true,
      .type_required = false,
      .function_type_required = false,
      .affects_type_identity  = false,
      .handler = &attribute_handlers::bitpack_stat_category,
      .exclude = NULL
   };
   static struct attribute_spec bitpack_string = {
      .name = "lu_bitpack_string",
      .min_length = 0, // min argcount
      .max_length = 0, // max argcount
      .decl_required = false,
      .type_required = false,
      .function_type_required = false,
      .affects_type_identity  = true,
      .handler = &attribute_handlers::bitpack_string,
      .exclude = NULL
   };
   static struct attribute_spec bitpack_transforms = {
      .name = "lu_bitpack_transforms",
      .min_length = 1, // min argcount
      .max_length = 3, // max argcount // 3, for internal use
      .decl_required = false,
      .type_required = false,
      .function_type_required = false,
      .affects_type_identity  = true,
      .handler = &attribute_handlers::bitpack_transforms,
      .exclude = NULL
   };
   static struct attribute_spec bitpack_union_external_tag = {
      .name = "lu_bitpack_union_external_tag",
      .min_length = 1, // min argcount
      .max_length = 1, // max argcount
      .decl_required = true,
      .type_required = false,
      .function_type_required = false,
      .affects_type_identity  = true,
      .handler = &attribute_handlers::bitpack_union_external_tag,
      .exclude = NULL
   };
   static struct attribute_spec bitpack_union_internal_tag = {
      .name = "lu_bitpack_union_internal_tag",
      .min_length = 1, // min argcount
      .max_length = 1, // max argcount
      .decl_required = false,
      .type_required = false,
      .function_type_required = false,
      .affects_type_identity  = true,
      .handler = &attribute_handlers::bitpack_union_internal_tag,
      .exclude = NULL
   };
   static struct attribute_spec bitpack_union_member_id = {
      .name = "lu_bitpack_union_member_id",
      .min_length = 1, // min argcount
      .max_length = 1, // max argcount
      .decl_required = true,
      .type_required = false,
      .function_type_required = false,
      .affects_type_identity  = true,
      .handler = &attribute_handlers::bitpack_tagged_id,
      .exclude = NULL
   };
}

static void register_attributes(void* event_data, void* user_data) {
   register_attribute(&_attributes::test_attribute);
   register_attribute(&_attributes::nonstring);
   register_attribute(&_attributes::internal_invalid);
   register_attribute(&_attributes::internal_invalid_by_name);
   register_attribute(&_attributes::bitpack_as_opaque_buffer);
   register_attribute(&_attributes::bitpack_bitcount);
   register_attribute(&_attributes::bitpack_default_value);
   register_attribute(&_attributes::bitpack_omit);
   register_attribute(&_attributes::bitpack_range);
   register_attribute(&_attributes::bitpack_stat_category);
   register_attribute(&_attributes::bitpack_string);
   register_attribute(&_attributes::bitpack_transforms);
   register_attribute(&_attributes::bitpack_union_external_tag);
   register_attribute(&_attributes::bitpack_union_internal_tag);
   register_attribute(&_attributes::bitpack_union_member_id);
}

#include "pragma_handlers/debug_dump_bp_data_options.h"
#include "pragma_handlers/debug_dump_function.h"
#include "pragma_handlers/debug_dump_identifier.h"
#include "pragma_handlers/enable.h"
#include "pragma_handlers/generate_functions.h"
#include "pragma_handlers/serialized_offset_to_constant.h"
#include "pragma_handlers/serialized_sector_id_to_constant.h"
#include "pragma_handlers/set_options.h"

static void register_pragmas(void* event_data, void* user_data) {
   c_register_pragma_with_expansion(
      "lu_bitpack",
      "debug_dump_bp_data_options",
      &pragma_handlers::debug_dump_bp_data_options
   );
   c_register_pragma_with_expansion(
      "lu_bitpack",
      "debug_dump_function",
      &pragma_handlers::debug_dump_function
   );
   c_register_pragma_with_expansion(
      "lu_bitpack",
      "debug_dump_identifier",
      &pragma_handlers::debug_dump_identifier
   );
   c_register_pragma_with_expansion(
      "lu_bitpack",
      "enable",
      &pragma_handlers::enable
   );
   c_register_pragma_with_expansion(
      "lu_bitpack",
      "generate_functions",
      &pragma_handlers::generate_functions
   );
   c_register_pragma_with_expansion(
      "lu_bitpack",
      "serialized_offset_to_constant",
      &pragma_handlers::serialized_offset_to_constant
   );
   c_register_pragma_with_expansion(
      "lu_bitpack",
      "serialized_sector_id_to_constant",
      &pragma_handlers::serialized_sector_id_to_constant
   );
   c_register_pragma_with_expansion(
      "lu_bitpack",
      "set_options",
      &pragma_handlers::set_options
   );
}

#include "gcc_wrappers/events/on_type_finished.h"

#include "basic_global_state.h"
#include "bitpacking/verify_bitpack_attributes_on_type_finished.h"

int plugin_init (
   struct plugin_name_args*   plugin_info,
   struct plugin_gcc_version* version
) {
   if (!plugin_default_version_check(version, &gcc_version)) {
      std::cerr << "Plug-in " << plugin_info->base_name << " is for version " <<
                   GCCPLUGIN_VERSION_MAJOR << "." <<
                   GCCPLUGIN_VERSION_MINOR << ".\n";
      if (gcc_version.basever) {
         std::cerr << "We appear to be running in version " << gcc_version.basever << ".\n";
      }
      return 1;
   }
   
   register_callback(
      plugin_info->base_name,
      PLUGIN_INFO,
      /* callback  */ NULL,
      /* user_data */ &_my_plugin_info
   );
   
   //std::cerr << "Loaded plug-in: " << plugin_info->base_name << ".\n";
   
   for(int i = 0; i < plugin_info->argc; ++i) {
      const auto& arg = plugin_info->argv[i];
      if (std::string_view(arg.key) == "xml-out") {
         auto& dst = basic_global_state::get().xml_output_path;
         dst = arg.value;
         continue;
      }
   }
   
   register_callback(
      plugin_info->base_name,
      PLUGIN_ATTRIBUTES,
      register_attributes,
      NULL
   );
   register_callback(
      plugin_info->base_name,
      PLUGIN_PRAGMAS,
      register_pragmas,
      NULL
   );
   {
      auto& mgr = gcc_wrappers::events::on_type_finished::get();
      mgr.initialize(plugin_info->base_name);
      mgr.add(
         "Valid8Ty",
         [](gcc_wrappers::type::base type) {
            if (!basic_global_state::get().enabled)
               return;
            // Some bitpacking attributes can only be verified when a TYPE is finished. 
            bitpacking::verify_bitpack_attributes_on_type_finished(type);
         }
      );
   }
   
   return 0;
}