#pragma once
#include <cstdint>
#include "gcc_wrappers/type/base.h"
#include "gcc_wrappers/type/integral.h"
#include "gcc_wrappers/type/pointer.h"
#include "lu/type_containers/array.h"
#include "lu/singleton.h"

namespace gcc_wrappers {
   namespace impl {
      using we_have_builtin_integer_types_for = lu::type_containers::array<
         int,
         int8_t,
         int16_t,
         int32_t,
         int64_t,
         uint8_t,
         uint16_t,
         uint32_t,
         uint64_t,
         intmax_t,
         uintmax_t,
         intptr_t,
         uintptr_t,
         ptrdiff_t,
         size_t,
         ssize_t
      >;
   }
   
   class builtin_types;
   class builtin_types : public lu::singleton<builtin_types> {
      protected:
         builtin_types();
         
      public:
         template<typename T>
         static constexpr const bool integer_type_ready_for = impl::we_have_builtin_integer_types_for::contains<T>;
   
      public:
         type::base     basic_bool;
         type::base     basic_char;
         type::integral basic_int;
         type::base     basic_void;
         type::integral int8;
         type::integral int16;
         type::integral int32;
         type::integral int64;
         type::integral uint8;
         type::integral uint16;
         type::integral uint32;
         type::integral uint64;
         
         type::integral intmax;
         type::integral uintmax;
         
         type::pointer  const_char_ptr; // const char*
         type::pointer  char_ptr; // char*
         
         type::pointer  const_void_ptr; // const void*
         type::pointer  void_ptr; // void*
         
         type::integral intptr;
         type::integral uintptr;
         type::integral ptrdiff;
         type::integral size;   // size_t
         type::integral ssize;  // ssize_t (signed size_t)
         
      public:
         type::integral smallest_integral_for(size_t bitcount, bool is_signed = false) const;
         
         template<typename T>
         type::optional_integral type_for() const requires integer_type_ready_for<T> {
            if constexpr (std::is_same_v<T, int>)
               return basic_int;
            if constexpr (std::is_same_v<T, int8_t>)
               return int8;
            if constexpr (std::is_same_v<T, int16_t>)
               return int16;
            if constexpr (std::is_same_v<T, int32_t>)
               return int32;
            if constexpr (std::is_same_v<T, int64_t>)
               return int64;
            if constexpr (std::is_same_v<T, uint8_t>)
               return uint8;
            if constexpr (std::is_same_v<T, uint16_t>)
               return uint16;
            if constexpr (std::is_same_v<T, uint32_t>)
               return uint32;
            if constexpr (std::is_same_v<T, uint64_t>)
               return uint64;
            if constexpr (std::is_same_v<T, intmax_t>)
               return intmax;
            if constexpr (std::is_same_v<T, uintmax_t>)
               return uintmax;
            if constexpr (std::is_same_v<T, intptr_t>)
               return intptr;
            if constexpr (std::is_same_v<T, uintptr_t>)
               return uintptr;
            if constexpr (std::is_same_v<T, ptrdiff_t>)
               return ptrdiff;
            if constexpr (std::is_same_v<T, size_t>)
               return size;
            if constexpr (std::is_same_v<T, ssize_t>)
               return ssize;
            return {};
         }
   };
}