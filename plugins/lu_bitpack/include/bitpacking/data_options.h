#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>
#include "lu/variants/contains_type.h"
#include "bitpacking/data_options/typed.h"
#include "gcc_wrappers/node.h"
#include "gcc_wrappers/optional.h"

namespace gcc_wrappers {
   namespace decl {
      class field;
      class param;
      class variable;
   }
   namespace type {
      class base;
   }
   class attribute_list;
}

namespace bitpacking {
   class data_options {
      protected:
         bool _loaded = false;
         bool _failed = false;
         bool _reported_typed_load_failure = false;
      
      public:
         struct {
            bool report_errors = true;
         } config;
         //
         gcc_wrappers::optional_node default_value;
         bool has_attr_nonstring = false;
         bool is_omitted         = false;
         std::vector<std::string> stat_categories;
         std::optional<intmax_t>  union_member_id;
         //
      protected:
         std::variant<
            typed_data_options::requested_variant, // intentionally the initial value
            typed_data_options::computed_variant
         > typed;
         
      protected:
         template<typename T>
         T& _get_or_emplace_for_load() {
            auto& var = std::get<typed_data_options::requested_variant>(this->typed);
            return std::holds_alternative<T>(var) ? std::get<T>(var) : var.emplace<T>();
         }
         
         void _report_typed_load_failure(gcc_wrappers::node target, gcc_wrappers::node node, std::string_view desired_type);
         
         // Returns `true` on failure.
         template<typename T>
         bool _fail_if_cannot_load_as(gcc_wrappers::node target, gcc_wrappers::node node, std::string_view desired) {
            auto& var = std::get<typed_data_options::requested_variant>(this->typed);
            if (std::holds_alternative<std::monostate>(var))
               return false;
            if (!std::holds_alternative<T>(var)) {
               _report_typed_load_failure(target, node, desired);
               return true;
            }
            return false;
         }
      
         void _load_contributing_entity(gcc_wrappers::node contributing_to, gcc_wrappers::node contributor, gcc_wrappers::attribute_list);
         void _finalize(gcc_wrappers::node);
         void _validate_union(gcc_wrappers::type::base);
         [[noreturn]] void _as_accessor_failed() const;
   
      public:
         constexpr bool valid() const noexcept {
            return !this->_failed;
         }
         
         void load(gcc_wrappers::decl::field);
         void load(gcc_wrappers::decl::param);
         void load(gcc_wrappers::decl::variable);
         void load(gcc_wrappers::type::base);
         
         template<typename T>
         constexpr bool is() const noexcept {
            if constexpr (lu::variants::contains_type<typed_data_options::requested_variant, T>) {
               return false;
            } else {
               if (std::holds_alternative<typed_data_options::requested_variant>(this->typed)) {
                  return false;
               }
               auto& var = std::get<typed_data_options::computed_variant>(this->typed);
               return std::holds_alternative<T>(var);
            }
         }
         
         template<typename T>
         constexpr const T& as() const noexcept {
            if (!is<T>())
               _as_accessor_failed();
            auto& var = std::get<typed_data_options::computed_variant>(this->typed);
            return std::get<T>(var);
         }
         
         // Compare only options that affect the serialization format (i.e. ignore 
         // things like default values and similar).
         bool same_format_as(const data_options&) const;
   };
}