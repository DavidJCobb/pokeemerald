#pragma once
#include <optional>
#include <variant>
#include "codegen/serialization_items/basic_segment.h"
#include "codegen/serialization_items/padding_segment.h"
#include "codegen/serialization_items/condition.h"

namespace codegen::serialization_items {
   class segment {
      public:
         std::variant<
            basic_segment,
            padding_segment
         > data;
         std::optional<condition_type> condition;
         
      public:
         bool operator==(const segment&) const noexcept = default;
         
      public:
         constexpr bool is_basic() const noexcept {
            return std::holds_alternative<basic_segment>(this->data);
         }
         constexpr bool is_padding() const noexcept {
            return std::holds_alternative<padding_segment>(this->data);
         }
         
         constexpr const basic_segment& as_basic() const noexcept {
            return std::get<basic_segment>(this->data);
         }
         constexpr const padding_segment& as_padding() const noexcept {
            return std::get<padding_segment>(this->data);
         }
         
         constexpr basic_segment& as_basic() noexcept {
            return const_cast<basic_segment&>(std::as_const(*this).as_basic());
         }
         constexpr padding_segment& as_padding() noexcept {
            return const_cast<padding_segment&>(std::as_const(*this).as_padding());
         }
   };
}