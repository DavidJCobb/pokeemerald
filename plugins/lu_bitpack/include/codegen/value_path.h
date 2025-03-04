#pragma once
#include <cstdint>
#include <variant>
#include <vector>
#include "codegen/decl_descriptor.h"
#include "codegen/decl_descriptor_pair.h"
#include "codegen/optional_value_pair.h"

namespace bitpacking {
   class data_options;
}

namespace codegen {
   class value_path_segment {
      public:
         std::variant<
            decl_descriptor_pair, // member access (or root variable)
            decl_descriptor_pair, // array access with a loop counter variable as the index
            size_t                // array access with an integer constant as the index
         > data;
         
      public:
         constexpr bool is_array_access() const {
            return this->data.index() > 0;
         }
         
         const decl_descriptor_pair descriptor() const {
            switch (this->data.index()) {
               case 0:
                  return member_descriptor();
               case 1:
                  return std::get<1>(this->data);
            }
            return {};
         }
         
         constexpr decl_descriptor_pair& member_descriptor() {
            return std::get<0>(this->data);
         }
         constexpr const decl_descriptor_pair& member_descriptor() const {
            return std::get<0>(this->data);
         }
         constexpr decl_descriptor_pair& array_loop_counter_descriptor() {
            return std::get<1>(this->data);
         }
         constexpr const decl_descriptor_pair& array_loop_counter_descriptor() const {
            return std::get<1>(this->data);
         }
         
      public:
         // needed by some files due to GCC header jank
         bool operator==(const value_path_segment&) const noexcept = default;
   };
   
   class value_path {
      public:
         std::vector<value_path_segment> segments;
         
      public:
         void access_array_element(decl_descriptor_pair);
         void access_array_element(size_t);
         void access_member(const decl_descriptor&);
         
         void replace(decl_descriptor_pair);
         
         optional_value_pair as_value_pair() const;
         const bitpacking::data_options& bitpacking_options() const;
         
      public:
         // needed by some files due to GCC header jank
         bool operator==(const value_path&) const noexcept = default;
   };
}