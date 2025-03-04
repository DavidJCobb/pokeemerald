#pragma once
#include <functional>
#include <string_view>

namespace lu::strings {
   struct kv_string_param {
      std::string_view name;
      bool has_param = false;
      bool int_param = false;
      std::function<void(std::string_view text, int number)> handler;
   };

   namespace _impl {
      extern void handle_kv_string(std::string_view data, const kv_string_param* params, size_t param_count);
   }

   // Throws std::runtime_error on invalid strings.
   template<size_t Count>
   void handle_kv_string(std::string_view data, const std::array<kv_string_param, Count>& params) {
      return _impl::handle_kv_string(data, params.data(), params.size());
   }
}
