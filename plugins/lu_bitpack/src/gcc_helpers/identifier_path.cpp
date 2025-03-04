#include <limits>
#include "gcc_helpers/identifier_path.h"
#include "lu/stringf.h"
#include <c-family/c-common.h> // lookup_name
#include "pragma_parse_exception.h"

#include "gcc_wrappers/decl/base_value.h"
#include "gcc_wrappers/type/base.h"
#include "gcc_wrappers/type/container.h"
#include "gcc_wrappers/type/pointer.h"
namespace gw {
   using namespace gcc_wrappers;
}

namespace gcc_helpers {
   void identifier_path::parse(cpp_reader& reader) {
      location_t loc;
      tree       data;
      do {
         cpp_ttype segment_token = (cpp_ttype)-1;
         cpp_ttype token_type    = pragma_lex(&data, &loc);
         if (token_type == CPP_EOF)
            break;
         if (!this->path.empty()) {
            switch (token_type) {
               case CPP_DEREF:
               case CPP_DOT:
               case CPP_SCOPE:
                  segment_token = token_type;
                  break;
               default:
                  throw pragma_parse_exception(
                     loc,
                     "expected %<.%>, %<->%>, %<::%>, or end of line, after %<%s%>",
                     this->path.back().id.name().c_str()
                  );
            }
            token_type = pragma_lex(&data, &loc);
         }
         if (token_type != CPP_NAME) {
            const auto* tn = cpp_type2name(token_type, 0);
            if (this->path.empty()) {
               throw pragma_parse_exception(loc, "expected an identifier; saw %s instead", tn);
            }
            throw pragma_parse_exception(
               loc,
               "expected an identifier after %<%s%>; saw %s instead",
               this->path.back().id.name().c_str(),
               tn
            );
         }
         this->path.emplace_back(segment{
            .token = segment_token,
            .id    = gw::identifier::wrap(data),
         });
      } while (true);
   }
   
   std::string identifier_path::_fully_qualified_name(size_t up_to) const {
      size_t end = this->path.size();
      if (end > up_to)
         end = up_to;
      
      std::string fqn;
      for(size_t i = 0; i < end; ++i) {
         auto& segm = this->path[i];
         if (segm.token != (cpp_ttype)-1) {
            fqn += cpp_type2name(segm.token, 0);
         }
         fqn += segm.id.name();
      }
      return fqn;
   }
   
   gw::optional_node identifier_path::resolve() const {
      if (this->path.empty()) {
         throw std::runtime_error("no identifier specified");
      }
      
      gw::optional_node subject;
      
      //
      // Start with the top-level identifier.
      //
      {
         auto id = this->path.front().id;
         subject = lookup_name(id.unwrap());
         if (!subject) {
            //
            // Handle types that have no accompanying DECL node (i.e. types 
            // whose names are in the tag namespace).
            //
            subject = identifier_global_tag(id.unwrap());
            if (!subject) {
               throw std::runtime_error(lu::stringf("identifier %<%s%> not found", id.name().c_str()));
            }
         }
      }
      //
      // Resolve nested names.
      //
      for(size_t i = 1; i < this->path.size(); ++i) {
         const auto& segm = this->path[i];
         
         auto _validate_operator = [this, &segm, i](cpp_ttype desired, gw::type::base type) {
            if (segm.token == desired)
               return;
            auto fqn = this->_fully_qualified_name(i);
            throw std::runtime_error(lu::stringf(
               "cannot access members of %<%s%> (type %<%s%>) using the %s operator (use the %s operator instead)",
               fqn.c_str(),
               type.name().data(),
               cpp_type2name(segm.token, 0),
               cpp_type2name(desired, 0)
            ));
         };
         
         gw::type::optional_container cont;
         if (subject->is<gw::type::container>()) {
            auto type = subject->as<gw::type::container>();
            _validate_operator(CPP_SCOPE, type);
            cont = type;
         } else if (subject->is<gw::decl::base_value>()) {
            auto decl = subject->as<gw::decl::base_value>();
            auto type = decl.value_type();
            {
               auto desired = CPP_DOT;
               if (type.is<gw::type::pointer>()) {
                  type = type.remove_pointer();
                  desired = CPP_DEREF;
               }
               _validate_operator(desired, type);
            }
            if (!type.is<gw::type::container>()) {
               auto fqn = this->_fully_qualified_name(i);
               throw std::runtime_error(lu::stringf(
                  "cannot access members of value %<%s%> because its type %<%s%> is not a struct or union",
                  fqn.c_str(),
                  type.name().data()
               ));
            }
            cont = type.as_container();
         } else {
            auto fqn = this->_fully_qualified_name(i);
            throw std::runtime_error(lu::stringf(
               "cannot access members of declaration %<%s%> because it is not a value of struct or union type",
               fqn.c_str()
            ));
         }
         
         gw::optional_node memb;
         const auto        memb_name = this->path[i].id.name();
         for(auto item : cont->member_chain()) {
            if (item.is<gw::decl::base>()) {
               auto cast = item.as<gw::decl::base>();
               auto name = cast.name();
               if (name == memb_name) {
                  memb = cast;
                  break;
               }
            } else if (item.is<gw::type::container>()) {
               auto cast = item.as<gw::type::container>();
               auto name = cast.name();
               if (name == memb_name) {
                  memb = cast;
                  break;
               }
            }
         }
         if (!memb) {
            auto fqn = this->_fully_qualified_name(i);
            throw std::runtime_error(lu::stringf(
               "expression %<%s%> refers to a struct or union which does not contain a member (static or non-static) named %<%s%>",
               fqn.c_str(),
               segm.id.name().data()
            ));
         }
         subject = memb;
      }
      
      return subject;
   }
   
   std::string identifier_path::fully_qualified_name() const {
      return this->_fully_qualified_name(std::numeric_limits<size_t>::max());
   }
}