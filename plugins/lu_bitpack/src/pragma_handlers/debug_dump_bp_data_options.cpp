#include "pragma_handlers/debug_dump_identifier.h"
#include <iostream>

#include <gcc-plugin.h>
#include <tree.h>
#include <c-family/c-common.h> // lookup_name

#include "bitpacking/data_options.h"

#include "gcc_wrappers/decl/base_value.h"
#include "gcc_wrappers/type/base.h"
#include "gcc_wrappers/type/container.h"
#include "gcc_wrappers/identifier.h"
#include "gcc_wrappers/scope.h"
namespace gw {
   using namespace gcc_wrappers;
}

#include "gcc_helpers/identifier_path.h"
#include "pragma_parse_exception.h"
#include "lu/stringf.h"

constexpr const char* this_pragma_name = "#pragma lu_bitpack debug_dump_bp_data_options";

namespace pragma_handlers { 
   extern void debug_dump_bp_data_options(cpp_reader* reader) {
      gw::optional_node subject;
      std::string fqn;
      {
         gcc_helpers::identifier_path path;
         try {
            path.parse(*reader);
         } catch (pragma_parse_exception& ex) {
            auto out = lu::stringf("%s: failed: %s", this_pragma_name, ex.what());
            inform(ex.location, out.c_str());
            return;
         }
         fqn = path.fully_qualified_name();
         try {
            subject = path.resolve();
         } catch (std::runtime_error& ex) {
            auto out = lu::stringf("%s: failed to dump identifier %<%s%>: %s", this_pragma_name, fqn.c_str(), ex.what());
            inform(UNKNOWN_LOCATION, out.c_str());
            return;
         }
      }
      if (!subject) { // should never happen
         inform(UNKNOWN_LOCATION, "identifier not found");
         return;
      }
      
      bitpacking::data_options options;
      options.config.report_errors = false;
      if (subject->is<gw::type::base>()) {
         options.load(subject->as<gw::type::base>());
      } else if (subject->is<gw::decl::field>()) {
         options.load(subject->as<gw::decl::field>());
      } else if (subject->is<gw::decl::variable>()) {
         options.load(subject->as<gw::decl::variable>());
      } else {
         std::cerr << "error: " << this_pragma_name << ": identifier " << fqn << " does not appear to be something that can have bitpacking data options\n";
         return;
      }
      
      std::cerr << "Printing data options for " << fqn << "...\n";
      if (!options.valid())
         std::cerr << "(Note: options were not valid.)\n";
      
      if (options.default_value) {
         std::cerr << " - Default value of type " << options.default_value->code_name() << '\n';
      }
      if (options.is_omitted) {
         std::cerr << " - Omitted from bitpacking\n";
      }
      if (!options.stat_categories.empty()) {
         std::cerr << " - Stat-tracking categories:\n";
         for(const auto& name : options.stat_categories)
            std::cerr << "    - " << name << '\n';
      }
      if (options.union_member_id.has_value()) {
         std::cerr << " - Union member ID: " << *options.union_member_id << '\n';
      }
      
      //
      // Print typed options.
      //
      
      if (options.is<bitpacking::typed_data_options::computed::boolean>()) {
         std::cerr << " - Bitpacking type: boolean\n";
      } else if (options.is<bitpacking::typed_data_options::computed::buffer>()) {
         const auto& src = options.as<bitpacking::typed_data_options::computed::buffer>();
         std::cerr << " - Bitpacking type: opaque buffer\n";
         std::cerr << "    - Bytecount: " << src.bytecount << '\n';
      } else if (options.is<bitpacking::typed_data_options::computed::integral>()) {
         const auto& src = options.as<bitpacking::typed_data_options::computed::integral>();
         std::cerr << " - Bitpacking type: integral\n";
         std::cerr << "    - Bitcount: " << src.bitcount << '\n';
         std::cerr << "    - Min:      " << src.min << '\n';
         if (src.max == bitpacking::typed_data_options::computed::integral::no_maximum) {
            std::cerr << "    - Max:      <none>\n";
         } else {
            std::cerr << "    - Max:      " << src.max << '\n';
         }
      } else if (options.is<bitpacking::typed_data_options::computed::pointer>()) {
         std::cerr << " - Bitpacking type: pointer\n";
      } else if (options.is<bitpacking::typed_data_options::computed::string>()) {
         const auto& src = options.as<bitpacking::typed_data_options::computed::string>();
         std::cerr << " - Bitpacking type: string\n";
         std::cerr << "    - Length:    " << src.length << '\n';
         std::cerr << "    - Nonstring: " << (src.nonstring ? "yes" : "no") << '\n';
      } else if (options.is<bitpacking::typed_data_options::computed::structure>()) {
         std::cerr << " - Bitpacking type: struct\n";
      } else if (options.is<bitpacking::typed_data_options::computed::tagged_union>()) {
         const auto& src = options.as<bitpacking::typed_data_options::computed::tagged_union>();
         std::cerr << " - Bitpacking type: union\n";
         std::cerr << "    - Tag identifier:    " << src.tag_identifier << '\n';
         std::cerr << "    - Internally tagged: " << (src.is_internal ? "yes" : "no") << '\n';
      } else if (options.is<bitpacking::typed_data_options::computed::transformed>()) {
         const auto& src = options.as<bitpacking::typed_data_options::computed::transformed>();
         std::cerr << " - Bitpacking type: transformed\n";
         {
            std::cerr << "    - Transformed type: ";
            if (src.transformed_type) {
               auto name = src.transformed_type->name();
               if (name.empty())
                  std::cerr << "<unnamed>";
               else
                  std::cerr << name;
            } else {
               std::cerr << "<none>";
            }
            std::cerr << '\n';
         }
         {
            std::cerr << "    - Pack function: ";
            if (src.pre_pack) {
               auto name = src.pre_pack->name();
               if (name.empty())
                  std::cerr << "<unnamed>";
               else
                  std::cerr << name;
            } else {
               std::cerr << "<none>";
            }
            std::cerr << '\n';
         }
         {
            std::cerr << "    - Unpack function: ";
            if (src.post_unpack) {
               auto name = src.post_unpack->name();
               if (name.empty())
                  std::cerr << "<unnamed>";
               else
                  std::cerr << name;
            } else {
               std::cerr << "<none>";
            }
            std::cerr << '\n';
         }
         {
            std::cerr << "    - Allow splitting across sectors: ";
            if (src.never_split_across_sectors)
               std::cerr << "no";
            else
               std::cerr << "yes";
            std::cerr << '\n';
         }
      } else {
         std::cerr << " - Bitpacking type: unknown\n";
      }
      
      //
      // TODO: Print where options are inherited from (e.g. type(def)s)?
      //
      
      
   }
}