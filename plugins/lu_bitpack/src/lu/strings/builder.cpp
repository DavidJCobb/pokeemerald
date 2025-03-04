#include "lu/strings/builder.h"
#include "lu/variants/contains_type.h"
#include <stdexcept>

namespace lu::strings {
   size_t builder::_entry_size(const entry& e) const {
      if (std::holds_alternative<std::string>(e)) {
         return std::get<std::string>(e).size();
      } else if (std::holds_alternative<std::string_view>(e)) {
         return std::get<std::string_view>(e).size();
      }
      return 0;
   }
   builder::entry builder::_list_to_entry() {
      auto item = std::visit(
         [](auto& item) -> entry {
            using visited_type = std::decay_t<decltype(item)>;
            if constexpr (variants::contains_type<entry, visited_type>) {
               return std::move(item);
            } else {
               return {};
            }
         },
         this->_list
      );
      this->_list = {};
      return item;
   }
   
   void builder::append(const std::string& item) {
      if (item.empty())
         return;
      _append(item);
   }
   void builder::append(std::string_view item) {
      if (item.empty())
         return;
      _append(item);
   }
   
   void builder::prepend(const std::string& item) {
      if (item.empty())
         return;
      _prepend(item);
   }
   void builder::prepend(std::string_view item) {
      if (item.empty())
         return;
      _prepend(item);
   }
   
   // number of strings to concatenate
   size_t builder::count() const {
      if (std::holds_alternative<entry_list>(this->_list)) {
         return std::get<entry_list>(this->_list).size();
      }
      if (std::holds_alternative<std::monostate>(this->_list)) {
         return 0;
      }
      return 1;
   }
   
   // size of the result string
   size_t builder::size() const {
      size_t s = 0;
      if (std::holds_alternative<entry_list>(this->_list)) {
         for(const auto& entry : std::get<entry_list>(this->_list)) {
            s += _entry_size(entry);
         }
      } else if (std::holds_alternative<std::string>(this->_list)) {
         s = std::get<std::string>(this->_list).size();
      } else if (std::holds_alternative<std::string_view>(this->_list)) {
         s = std::get<std::string_view>(this->_list).size();
      }
      return s;
   }
   
   // remove N-th string. throws std::out_of_range on failure
   void builder::remove(size_t n) {
      constexpr const char* const message = "out-of-range removal from lu::strings::builder";
      
      if (std::holds_alternative<entry_list>(this->_list)) {
         auto& list = std::get<entry_list>(this->_list);
         auto  it   = list.begin();
         if (it == list.end())
            throw std::out_of_range(message);
         for(size_t i = 0; i < n; ++i, ++it)
            if (it == list.end())
               throw std::out_of_range(message);
         list.erase(it);
         return;
      }
      if (n > 0) {
         throw std::out_of_range(message);
      }
      if (std::holds_alternative<std::monostate>(this->_list)) {
         throw std::out_of_range(message);
      }
      this->_list = {};
   }
   
   std::string builder::build() const {
      if (empty()) {
         return {};
      }
      if (std::holds_alternative<std::string>(this->_list)) {
         return std::get<std::string>(this->_list);
      }
      if (std::holds_alternative<std::string_view>(this->_list)) {
         return std::string(std::get<std::string_view>(this->_list));
      }
      
      auto& list = std::get<entry_list>(this->_list);
      
      std::string output;
      {
         size_t s = 0;
         for(const auto& item : list)
            s += _entry_size(item);
         output.reserve(s);
      }
      for(const auto& item : list) {
         std::visit(
            [&output](const auto& v) {
               output += v;
            },
            item
         );
      }
      return output;
   }
   
   std::string builder::build_and_clear() {
      if (empty()) {
         return {};
      }
      
      std::string output;
      if (std::holds_alternative<std::string>(this->_list)) {
         output = std::move(std::get<std::string>(this->_list));
         this->_list = {};
         return output;
      }
      if (std::holds_alternative<std::string_view>(this->_list)) {
         output = std::move(std::get<std::string_view>(this->_list));
         this->_list = {};
         return output;
      }
      
      auto& list = std::get<entry_list>(this->_list);
      {
         size_t s = 0;
         for(const auto& item : list)
            s += _entry_size(item);
         output.reserve(s);
      }
      for(auto& item : list) {
         std::visit(
            [&output](auto& v) {
               output += v;
               v = {};
            },
            item
         );
      }
      this->_list = {};
      return output;
   }
}