#pragma once
#include <memory>
#include <type_traits>
#include <vector>
#include "codegen/expr_pair.h"

namespace codegen::instructions::utils {
   struct generation_context;
}

namespace codegen::instructions {
   enum class type {
      array_slice,
      container,
      padding,
      single,
      transform,
      union_case,
      union_switch,
   };
   
   class container;
   class union_switch;
   class base {
      public:
         virtual ~base() {}
         virtual type get_type() const noexcept {return (type)-1;}
         virtual expr_pair generate(const utils::generation_context&) const { assert(false && "abstract"); }
         
      public:
         template<typename Subclass> requires std::is_base_of_v<base, Subclass>
         const Subclass* as() const noexcept {
            auto t = this->get_type();
            if (t == Subclass::node_type)
               return (const Subclass*) this;
            if constexpr (std::is_same_v<std::decay_t<Subclass>, container>) {
               switch (t) {
                  case type::array_slice:
                  case type::transform:
                  case type::union_case:
                     return (const container*) this;
                  default:
                     break;
               }
            }
            return nullptr;
         }
         
         template<typename Subclass> requires std::is_base_of_v<base, Subclass>
         Subclass* as() noexcept {
            return const_cast<Subclass*>(std::as_const(*this).as<Subclass>());
         }
   };
   
   class container : public base {
      public:
         static constexpr const type node_type = type::container;
         virtual type get_type() const noexcept override { return node_type; };
         
         virtual expr_pair generate(const utils::generation_context&) const;
         
      public:
         std::vector<std::unique_ptr<base>> instructions;
   };
}