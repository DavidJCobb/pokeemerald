#pragma once
#include "lu/strings/zview.h"

namespace lu::strings {
   constexpr bool zview::starts_with(std::string_view s) const {
      return std::string_view(*this).starts_with(s);
   }
   constexpr bool zview::starts_with(value_type c) const {
      return front() == c;
   }
   
   constexpr bool zview::ends_with(std::string_view s) const {
      return std::string_view(*this).ends_with(s);
   }
   constexpr bool zview::ends_with(value_type c) const {
      return back() == c;
   }
   
   #if __cpp_lib_string_contains >= 202011L
   constexpr bool zview::contains(std::string_view s) const {
      return std::string_view(*this).contains(s);
   }
   constexpr bool zview::contains(value_type c) const {
      return std::string_view(*this).contains(c);
   }
   #endif
   
   constexpr zview::size_type zview::find(std::string_view needle, size_type pos) const noexcept {
      return std::string_view(*this).find(needle, pos);
   }
   constexpr zview::size_type zview::find(value_type needle, size_type pos) const noexcept {
      return std::string_view(*this).find(needle, pos);
   }
   constexpr zview::size_type zview::find(const value_type* needle, size_type pos, size_type needle_size) const noexcept {
      return std::string_view(*this).find(needle, pos, needle_size);
   }
   
   constexpr zview::size_type zview::rfind(std::string_view needle, size_type pos) const noexcept {
      return std::string_view(*this).rfind(needle, pos);
   }
   constexpr zview::size_type zview::rfind(value_type needle, size_type pos) const noexcept {
      return std::string_view(*this).rfind(needle, pos);
   }
   constexpr zview::size_type zview::rfind(const value_type* needle, size_type pos, size_type needle_size) const noexcept {
      return std::string_view(*this).rfind(needle, pos, needle_size);
   }
         
   constexpr zview::size_type zview::find_first_of(std::string_view set, size_type pos) const noexcept {
      return std::string_view(*this).find_first_of(set, pos);
   }
   constexpr zview::size_type zview::find_first_of(value_type c, size_type pos) const noexcept {
      return std::string_view(*this).find_first_of(c, pos);
   }
   constexpr zview::size_type zview::find_first_of(const_pointer set, size_type pos, size_type set_size) const noexcept {
      return std::string_view(*this).find_first_of(set, pos, set_size);
   }
         
   constexpr zview::size_type zview::find_last_of(std::string_view set, size_type pos) const noexcept {
      return std::string_view(*this).find_last_of(set, pos);
   }
   constexpr zview::size_type zview::find_last_of(value_type c, size_type pos) const noexcept {
      return std::string_view(*this).find_last_of(c, pos);
   }
   constexpr zview::size_type zview::find_last_of(const_pointer set, size_type pos, size_type set_size) const noexcept {
      return std::string_view(*this).find_last_of(set, pos, set_size);
   }
         
   constexpr zview::size_type zview::find_first_not_of(std::string_view set, size_type pos) const noexcept {
      return std::string_view(*this).find_first_not_of(set, pos);
   }
   constexpr zview::size_type zview::find_first_not_of(value_type c, size_type pos) const noexcept {
      return std::string_view(*this).find_first_not_of(c, pos);
   }
   constexpr zview::size_type zview::find_first_not_of(const_pointer set, size_type pos, size_type set_size) const noexcept {
      return std::string_view(*this).find_first_not_of(set, pos, set_size);
   }
         
   constexpr zview::size_type zview::find_last_not_of(std::string_view set, size_type pos) const noexcept {
      return std::string_view(*this).find_last_not_of(set, pos);
   }
   constexpr zview::size_type zview::find_last_not_of(value_type c, size_type pos) const noexcept {
      return std::string_view(*this).find_last_not_of(c, pos);
   }
   constexpr zview::size_type zview::find_last_not_of(const_pointer set, size_type pos, size_type set_size) const noexcept {
      return std::string_view(*this).find_last_not_of(set, pos, set_size);
   }
}