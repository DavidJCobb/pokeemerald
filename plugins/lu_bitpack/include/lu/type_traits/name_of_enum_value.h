#pragma once
#include <array>
#include <string_view>
#include <type_traits>

namespace lu {
   namespace impl::_name_of_enum_value {
      template<auto Value>
      constexpr auto function_name() {
         return std::string_view{
            #if defined(__clang__) || defined(__GNUC__)
               __PRETTY_FUNCTION__
            #elif defined (_MSC_VER)
               __FUNCSIG__
            #endif
         };
      }

      template<auto Value>
      constexpr auto compute() {
         constexpr auto prefix = std::string_view(
            #if defined(__clang__)
               "[Value = "
            #elif defined(__GNUC__)
               "with auto Value = "
            #elif defined (_MSC_VER)
               "compute<"
            #endif
         );
         constexpr auto suffix = std::string_view(
            #if defined(__clang__)
               "]"
            #elif defined(__GNUC__)
               "]"
            #elif defined (_MSC_VER)
               ">("
            #endif
         );
         constexpr auto this_function = std::string_view{
            #if defined(__clang__) || defined(__GNUC__) //|| defined(__INTELLISENSE__)
               __PRETTY_FUNCTION__
            #elif defined (_MSC_VER)
               __FUNCSIG__
            #endif
         };

         constexpr size_t start = this_function.find(prefix) + prefix.size();
         constexpr size_t end   = this_function.rfind(suffix);
         static_assert(start < end);

         constexpr auto view  = this_function.substr(start, (end - start));

         std::array<char, (end - start + 1)> out = {};
         for (size_t i = 0; i < out.size(); ++i)
            out[i] = view.data()[i];
         out[out.size() - 1] = '\0';
         return out;
      }

      template<auto Value>
      struct storage {
         static constexpr const auto value = compute<Value>();
      };
   }

   template<auto Value> requires std::is_enum_v<std::decay_t<decltype(Value)>>
   constexpr std::string_view name_of_enum_value() {
      constexpr const auto& arr = impl::_name_of_enum_value::storage<Value>::value;
      return std::string_view(arr.data(), arr.size());
   }
}