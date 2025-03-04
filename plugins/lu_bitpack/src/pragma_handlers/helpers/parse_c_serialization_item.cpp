#include "pragma_handlers/helpers/parse_c_serialization_item.h"
#include <c-family/c-common.h> // lookup_name
#include <variant>
#include <vector>
#include "gcc_wrappers/constant/integer.h"
#include "gcc_wrappers/identifier.h"
#include "codegen/decl_descriptor.h"
#include "codegen/decl_dictionary.h"
#include "pragma_parse_exception.h"
namespace gw {
   using namespace gcc_wrappers;
}

namespace {
   struct array_access {
      size_t     index;
      location_t loc;
   };
   struct member_access {
      std::string identifier;
      location_t  loc;
   };
   
   struct head_node {
      std::string identifier;
      size_t      dereference_count = 0;
      location_t  loc;
   };
   using tail_node = std::variant<array_access, member_access>;
   
   struct parsed_path {
      head_node head;
      std::vector<tail_node> tail;
   };
}

namespace pragma_handlers::helpers {
   // May throw pragma_parse_exception.
   extern codegen::serialization_item parse_c_serialization_item(cpp_reader* reader) {
      parsed_path path;
      {
         location_t loc;
         tree       data = NULL_TREE;
         
         bool is_just_head = true;
         
         {  // Parse head node
            cpp_ttype token_type = pragma_lex(&data, &loc);
            if (token_type == CPP_OPEN_PAREN) {
               path.head.loc = loc;
               token_type = pragma_lex(&data, &loc);
               while (token_type == CPP_MULT) {
                  ++path.head.dereference_count;
                  token_type = pragma_lex(&data, &loc);
               }
               if (token_type == CPP_NAME) {
                  path.head.identifier = gw::identifier::wrap(data).name();
                  token_type = pragma_lex(&data, &loc);
                  if (token_type != CPP_CLOSE_PAREN) {
                     throw pragma_parse_exception(
                        loc,
                        "expected %<)%>; saw %<%s%> instead",
                        cpp_type2name(token_type, 0)
                     );
                  }
               } else {
                  throw pragma_parse_exception(
                     loc,
                     "expected a dereference operator %<*%> or an identifier; saw %<%s%> instead",
                     cpp_type2name(token_type, 0)
                  );
               }
            } else if (token_type == CPP_NAME) {
               path.head.loc = loc;
               path.head.identifier = gw::identifier::wrap(data).name();
            } else {
               throw pragma_parse_exception(
                  loc,
                  "expected an identifier, or something of the form %<(*identifier)%> given any number of dereference operators; saw %<%s%> instead",
                  cpp_type2name(token_type, 0)
               );
            }
            
            token_type = pragma_lex(&data, &loc);
            if (token_type == CPP_DOT) {
               is_just_head = false;
            } else if (token_type == CPP_DEREF) {
               is_just_head = false;
               ++path.head.dereference_count;
            } else if (token_type == CPP_EOF) {
               ;
            } else {
               throw pragma_parse_exception(
                  loc,
                  "unexpected %<%s%>",
                  cpp_type2name(token_type, 0)
               );
            }
         }
         
         if (!is_just_head) {  // Parse tail nodes
            cpp_ttype token_type = pragma_lex(&data, &loc);
            while (true) {
               auto tail_loc = loc;
               if (token_type == CPP_NAME) {
                  auto& variant = path.tail.emplace_back();
                  auto& node    = variant.emplace<member_access>();
                  node.identifier = gw::identifier::wrap(data).name();
                  node.loc        = tail_loc;
               } else if (token_type == CPP_OPEN_SQUARE) {
                  size_t index = 0;
                  
                  token_type = pragma_lex(&data, &loc);
                  if (token_type == CPP_NUMBER) {
                     if (TREE_CODE(data) != INTEGER_CST) {
                        throw pragma_parse_exception(
                           loc,
                           "expected a positive integer constant; saw %<%s%> instead",
                           cpp_type2name(token_type, 0)
                        );
                     }
                     auto node = gw::constant::integer::wrap(data);
                     if (node.sign() < 0) {
                        throw pragma_parse_exception(
                           loc,
                           "expected a positive integer constant; saw a negative integer instead"
                        );
                     }
                     auto index_opt = node.try_value_unsigned();
                     if (!index_opt.has_value()) {
                        throw pragma_parse_exception(
                           loc,
                           "expected a positive integer constant; got one, but could not read it (too large?)"
                        );
                     }
                     index = *index_opt;
                  } else {
                     throw pragma_parse_exception(
                        loc,
                        "expected a positive integer constant; saw %<%s%> instead",
                        cpp_type2name(token_type, 0)
                     );
                  }
                  token_type = pragma_lex(&data, &loc);
                  if (token_type != CPP_CLOSE_SQUARE) {
                     throw pragma_parse_exception(
                        loc,
                        "expected %<]%>; saw %<%s%> instead",
                        cpp_type2name(token_type, 0)
                     );
                  }
                  
                  auto& variant = path.tail.emplace_back();
                  auto& node    = variant.emplace<array_access>();
                  node.index = index;
                  node.loc   = tail_loc;
               } else if (token_type == CPP_EOF) {
                  throw pragma_parse_exception(loc, "unexpected end of expression");
               } else {
                  throw pragma_parse_exception(
                     loc,
                     "expected an identifier or array index; saw %<%s%> instead",
                     cpp_type2name(token_type, 0)
                  );
               }
               
               token_type = pragma_lex(&data, &loc);
               if (token_type == CPP_EOF) {
                  break;
               }
            }
         }
      }
      
      //
      // Given that parsed path, generate a serialization item.
      //
      codegen::serialization_item item;
      
      auto& dict = codegen::decl_dictionary::get();
      {  // Look up head identifier.
         auto id  = gw::identifier(path.head.identifier);
         auto raw = lookup_name(id.unwrap());
         if (raw == NULL_TREE) {
            throw pragma_parse_exception(
               path.head.loc,
               "identifier %<%s%> does not exist",
               path.head.identifier.c_str()
            );
         } else if (TREE_CODE(raw) != VAR_DECL) {
            throw pragma_parse_exception(
               path.head.loc,
               "identifier %<%s%> does not name a variable",
               path.head.identifier.c_str()
            );
         }
         
         auto  decl = gw::decl::variable::wrap(raw);
         auto* desc = dict.find_existing_descriptor(decl);
         if (!desc) {
            //
            // We never generated a descriptor for this variable, which means the 
            // codegen systems never even looked at it.
            //
            throw pragma_parse_exception(
               path.head.loc,
               "the requested value is not present in the serialized data",
               path.head.identifier.c_str()
            );
         }
         if (desc->variable.dereference_count != path.head.dereference_count) {
            if (desc->variable.dereference_count == 0) {
               if (!decl.value_type().is_pointer()) {
                  throw pragma_parse_exception(
                     path.head.loc,
                     "identifier %<%s%> is not a pointer and so cannot be dereferenced prior to serialization"
                  );
               }
               throw pragma_parse_exception(
                  path.head.loc,
                  "identifier %<%s%> is not dereferenced before serialization; you have requested %u dereferences",
                  path.head.dereference_count
               );
            }
            throw pragma_parse_exception(
               path.head.loc,
               "identifier %<%s%> is dereferenced %u times before serialization; you have requested %u dereferences",
               desc->variable.dereference_count,
               path.head.dereference_count
            );
         }
         
         item.append_segment(*desc);
      }
      for(auto& tail : path.tail) {
         if (std::holds_alternative<array_access>(tail)) {
            auto& data = std::get<array_access>(tail);
            auto& segm = item.segments.back().as_basic();
            if (!segm.is_array()) {
               throw pragma_parse_exception(
                  data.loc,
                  "this value is not an array; you cannot perform array access on it"
               );
            }
            auto extent = segm.array_extent();
            if (data.index >= extent) {
               throw pragma_parse_exception(
                  data.loc,
                  "cannot access array index %u, as the array has only %u items",
                  data.index,
                  extent
               );
            }
            segm.array_accesses.push_back({
               .start = data.index,
               .count = 1,
            });
         } else if (std::holds_alternative<member_access>(tail)) {
            auto& data = std::get<member_access>(tail);
            auto& segm = item.segments.back().as_basic();
            auto& desc = *segm.desc;
            if (segm.is_array()) {
               throw pragma_parse_exception(
                  data.loc,
                  "this value is an array; you cannot perform member access on it"
               );
            }
            if (!desc.types.serialized->is_container()) {
               throw pragma_parse_exception(
                  data.loc,
                  "this value is not a struct or union; you cannot perform member access on it"
               );
            }
            bool found = false;
            for(const auto* member_desc : desc.members_of_serialized()) {
               if (member_desc->decl.name() == data.identifier) {
                  found = true;
                  item.append_segment(*member_desc);
               }
            }
            if (!found) {
               throw pragma_parse_exception(
                  data.loc,
                  "the requested data member is not present in the to-be-serialized data"
               );
            }
         } else {
            assert(false && "unhandled case");
         }
      }
      
      return item;
   }
}