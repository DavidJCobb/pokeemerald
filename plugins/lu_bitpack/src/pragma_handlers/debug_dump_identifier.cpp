#include "pragma_handlers/debug_dump_identifier.h"
#include <iostream>
#include <string_view>

#include <gcc-plugin.h>
#include <tree.h>
#include <c-family/c-common.h> // lookup_name

#include <print-tree.h> // debug_tree

#include "gcc_wrappers/constant/floating_point.h"
#include "gcc_wrappers/constant/integer.h"
#include "gcc_wrappers/constant/string.h"
#include "gcc_wrappers/decl/base.h"
#include "gcc_wrappers/decl/field.h"
#include "gcc_wrappers/decl/function.h"
#include "gcc_wrappers/decl/type_def.h"
#include "gcc_wrappers/decl/variable.h"
#include "gcc_wrappers/type/base.h"
#include "gcc_wrappers/type/array.h"
#include "gcc_wrappers/type/enumeration.h"
#include "gcc_wrappers/type/integral.h"
#include "gcc_wrappers/attribute.h"
#include "gcc_wrappers/identifier.h"
#include "gcc_wrappers/scope.h"
#include <diagnostic.h>
namespace gw {
   using namespace gcc_wrappers;
}

#include "gcc_helpers/identifier_path.h"
#include "pragma_parse_exception.h"
#include "lu/stringf.h"

namespace pragma_handlers { 
   static void _print_bool(const std::string_view label, bool value, size_t indent = 0) {
      if (indent) {
         while (indent--)
            std::cerr << "   ";
      }
      std::cerr << '[';
      if (value) {
         std::cerr << 'x';
      } else {
         std::cerr << ' ';
      }
      std::cerr << ']';
      std::cerr << ' ';
      std::cerr << label;
      std::cerr << '\n';
   }

   extern void debug_dump_identifier(cpp_reader* reader) {
      constexpr const char* this_pragma_name = "#pragma lu_bitpack debug_dump_identifier";
      
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
      
      gw::optional_node also_dump;
      
      inform(UNKNOWN_LOCATION, "printing information about identifier %<%s%>", fqn.c_str());
      if (subject->is<gw::type::base>()) {
         std::cerr << "This object is a type.\n";
         
         auto type = subject->as<gw::type::base>();
         std::cerr << "Pretty-printed: " << type.pretty_print() << '\n';
         
         std::cerr << "Attributes:\n";
         auto list = type.attributes();
         if (list.empty()) {
            std::cerr << " - <none>\n";
         } else {
            for(auto attr : list) {
               std::cerr << " - ";
               std::cerr << attr.name();
               std::cerr << '\n';
            }
         }
         
         std::cerr << "Qualifiers:\n";
         _print_bool("Atomic", type.is_atomic());
         _print_bool("Const",  type.is_const());
         _print_bool("Restrict", type.is_restrict());
         _print_bool("Volatile", type.is_volatile());
         _print_bool("Allowed to be restrict", type.is_restrict_allowed());
         
         std::cerr << "Traits:\n";
         _print_bool("Is complete", type.is_complete());
         _print_bool("Is pointer", type.is_pointer());
         _print_bool("Is reference", type.is_reference());
         _print_bool("lvalue", type.is_lvalue_reference(), 1);
         _print_bool("rvalue", type.is_rvalue_reference(), 1);
         std::cerr << " - Alignment:    " << type.alignment_in_bits() << " bits (" << type.alignment_in_bytes() << " bytes)\n";
         std::cerr << " - Size: " << type.size_in_bits() << " bits (" << type.size_in_bytes() << " bytes)\n";
         std::cerr << " - Minimum required alignment: " << type.minimum_alignment_in_bits() << " bits\n";
         _print_bool("Is packed", type.is_packed());
         
         if (type.is_array()) {
            auto casted = type.as_array();
            
            std::cerr << "Array info:\n";
            std::cerr << " - Value type: " << (casted.value_type().pretty_print()) << "\n";
            {
               auto opt = casted.extent();
               std::cerr << " - Extent:     ";
               if (opt.has_value()) {
                  std::cerr << *opt;
               } else {
                  std::cerr << "unknown/VLA";
               }
               std::cerr << '\n';
            }
         } else if (type.is_enum()) {
            auto casted  = type.as_enum();
            
            std::cerr << "Enumeration info:\n";
            _print_bool("Is opaque", casted.is_opaque());
            _print_bool("Is scoped", casted.is_scoped());
            if (!casted.is_still_being_defined()) {
               auto under = casted.underlying_type();
               std::cerr << "Underlying type: " << under.pretty_print() << '\n';
            }
            
            auto members = casted.all_enum_members();
            std::cerr << "Enumeration members:\n";
            if (members.empty()) {
               std::cerr << " - <none>\n";
            } else {
               for(const auto& info : members) {
                  std::cerr << " - " << info.name << " == " << info.value << '\n';
               }
            }
         }
         if (type.is_integral()) {
            auto casted = type.as_integral();
            
            std::cerr << "Integral info:\n";
            _print_bool("Signed", casted.is_signed());
            std::cerr << " - Min: " << casted.minimum_value() << '\n';
            std::cerr << " - Max: " << casted.maximum_value() << '\n';
            std::cerr << " - Bitcount: " << casted.bitcount() << '\n';
         }
      } else if (subject->is<gw::decl::base>()) {
         std::cerr << "This object is a declaration.\n";
         
         auto decl = subject->as<gw::decl::base>();
         if (subject->is<gw::decl::type_def>()) {
            std::cerr << "That declaration is a typedef.\n";
            
            auto decl  = subject->as<gw::decl::type_def>();
            auto prior = decl.is_synonym_of();
            auto after = decl.declared();
            if (prior && after) {
               std::cerr << "Is a new synonym of: " << prior->pretty_print() << '\n';
               also_dump = after;
            } else {
               std::cerr << "Unable to get complete information. The typedef may still be in the middle of being parsed.\n";
            }
         } else {
            std::cerr << "Name: " << decl.name() << '\n';
         }
         
         std::cerr << "Attributes:\n";
         auto list = decl.attributes();
         if (list.empty()) {
            std::cerr << " - <none>\n";
         } else {
            for(auto attr : list) {
               std::cerr << " - ";
               std::cerr << attr.name();
               std::cerr << '\n';
            }
         }
         
         std::cerr << "Decl properties:\n";
         _print_bool("Artificial", decl.is_artificial());
         _print_bool("Ignored for debugging", decl.is_sym_debugger_ignored());
         _print_bool("Used", decl.is_used());
         
         auto scope = decl.context();
         if (scope) {
            if (scope->is_file_scope()) {
               std::cerr << "Scope: file\n";
            } else {
               auto nf = scope->nearest_function();
               if (nf) {
                  std::cerr << "Scope: function: ";
                  std::cerr << nf->name();
                  std::cerr << '\n';
               } else {
                  auto nt = scope->nearest_type();
                  if (nt) {
                     std::cerr << "Scope: type: ";
                     std::cerr << nt->pretty_print();
                     std::cerr << '\n';
                  }
               }
            }
         }
         
         if (subject->is<gw::decl::base_value>()) {
            auto decl = subject->as<gw::decl::base_value>();
            {
               gw::type::optional_base type;
               if (subject->is<gw::decl::field>()) {
                  //
                  // Value type access is overridden for fields, to account 
                  // for bitfields and how they affect typing.
                  //
                  auto decl = subject->as<gw::decl::field>();
                  type = decl.value_type();
               } else {
                  type = decl.value_type();
               }
               std::cerr << "Value type: " << type->pretty_print() << '\n';
            }
            if (subject->is<gw::decl::field>()) {
               auto decl = subject->as<gw::decl::field>();
               {
                  auto offset = decl.offset_in_bits();
                  if (offset % 8) {
                     std::cerr << "This field's offset is " << offset << " bits.\n";
                  } else {
                     std::cerr << "This field's offset is " << offset << " bits (" << (offset / 8) << " bytes).\n";
                  }
               }
               if (decl.is_bitfield()) {
                  std::cerr << "This field is a bitfield of width " << decl.size_in_bits() << ".\n";
               }
               if (decl.is_non_addressable()) {
                  std::cerr << "This field is not an addressable value.\n";
               }
               if (decl.is_padding()) {
                  std::cerr << "This field is compiler-generated padding.\n";
               }
            } else if (subject->is<gw::decl::variable>()) {
               auto decl = subject->as<gw::decl::variable>();
               if (decl.is_declared_constexpr()) {
                  std::cerr << "This variable was declared constexpr.\n";
               }
               auto init = decl.initial_value();
               if (init) {
                  std::cerr << "The declared entity has an initial value.\n";
                  if (init->is<gw::constant::integer>()) {
                     auto casted = init->as<gw::constant::integer>();
                     auto value  = casted.try_value_signed();
                     std::cerr << "Initial value: integer ";
                     if (value.has_value()) {
                        std::cerr << *value;
                     } else {
                        auto value_u = casted.try_value_unsigned();
                        if (value_u.has_value()) {
                           std::cerr << *value_u;
                        } else {
                           std::cerr << "<magnitude too large to print?>";
                        }
                     }
                     std::cerr << '\n';
                  } else if (init->is<gw::constant::floating_point>()) {
                     auto casted = init->as<gw::constant::floating_point>();
                     std::cerr << "Initial value: floating-point " << casted.to_string() << '\n';
                  } else if (init->is<gw::constant::string>()) {
                     auto casted = init->as<gw::constant::string>();
                     std::cerr << "Initial value: string[";
                     std::cerr << casted.length();
                     std::cerr << "]: ";
                     std::cerr << casted.value();
                     std::cerr << '\n';
                  }
               }
            }
         } else if (subject->is<gw::decl::function>()) {
            auto decl = subject->as<gw::decl::function>();
            std::cerr << "This is a function.\n";
            
            _print_bool("Noreturn", decl.is_noreturn());
            _print_bool("Nothrow", decl.is_nothrow());
            _print_bool("Is defined", decl.has_body());
            std::cerr << "Linkage:\n";
            if (decl.is_externally_accessible()) {
               std::cerr << " - Non-static\n";
            } else {
               std::cerr << " - Static\n";
            }
            if (decl.is_defined_elsewhere()) {
               std::cerr << " - Extern?\n";
            } else {
               std::cerr << " - Non-extern\n";
            }
         }
      }
      
      std::cerr << '\n';
      std::cerr << "Dumping identifier " << fqn << " as raw data...\n";
      debug_tree(subject.unwrap());
      
      if (also_dump) {
         std::cerr << "\nDumping typedef-declared type as raw data...\n";
         debug_tree(also_dump.unwrap());
      }
   }
}