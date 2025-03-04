#include "gcc_wrappers/builtin_types.h"
#include <cassert>
#include <gcc-plugin.h>
#include <tree.h> // *_type_node
#include <c-family/c-common.h> // *_type_node

namespace gcc_wrappers {
   builtin_types::builtin_types()
   :
      basic_bool([]() {
         assert(boolean_type_node != NULL_TREE && "You must be careful not to create this singleton too early, i.e. before GCC has initialized the built-in C types.");
         return type::base::wrap(boolean_type_node);
      }()),
      basic_char(type::base::wrap(char_type_node)),
      basic_int (type::integral::wrap(integer_type_node)),
      basic_void(type::base::wrap(void_type_node)),
      //
      int8  (type::integral::wrap(int8_type_node)),
      int16 (type::integral::wrap(int16_type_node)),
      int32 (type::integral::wrap(int32_type_node)),
      int64 (type::integral::wrap(int64_type_node)),
      uint8 (type::integral::wrap(uint8_type_node)),
      uint16(type::integral::wrap(c_uint16_type_node)),
      uint32(type::integral::wrap(c_uint32_type_node)),
      uint64(type::integral::wrap(c_uint64_type_node)),
      
      intmax (type::integral::wrap(intmax_type_node)),
      uintmax(type::integral::wrap(uintmax_type_node)),
      
      const_char_ptr(type::pointer::wrap(const_string_type_node)),
      char_ptr      (type::pointer::wrap(string_type_node)),
      
      const_void_ptr(type::pointer::wrap(const_ptr_type_node)),
      void_ptr      (type::pointer::wrap(ptr_type_node)),
      
      intptr (type::integral::wrap(intptr_type_node)),
      uintptr(type::integral::wrap(uintptr_type_node)),
      ptrdiff(type::integral::wrap(ptrdiff_type_node)),
      size   (type::integral::wrap(size_type_node)),
      ssize  (type::integral::wrap(signed_size_type_node))
   {}
   
   type::integral builtin_types::smallest_integral_for(size_t bitcount, bool is_signed) const {
      if (bitcount > 32)
         return is_signed ? int64 : uint64;
      if (bitcount > 16)
         return is_signed ? int32 : uint32;
      if (bitcount > 8)
         return is_signed ? int16 : uint16;
      return is_signed ? int8 : uint8;
   }
}