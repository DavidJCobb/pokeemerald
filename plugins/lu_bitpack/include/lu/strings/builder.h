#pragma once
#include <list>
#include <string>
#include <string_view>
#include <variant>

namespace lu::strings {
   class builder {
      protected:
         using entry      = std::variant<std::string, std::string_view>;
         using entry_list = std::list<entry>;
      
         std::variant<
            std::monostate,
            std::string,
            std::string_view,
            entry_list
         > _list;
         
      protected:
         size_t _entry_size(const entry&) const;
         entry _list_to_entry();
      
         template<typename T>
         void _append(const T& item) {
            if (std::holds_alternative<std::monostate>(this->_list)) {
               this->_list = item;
               return;
            }
            if (std::holds_alternative<entry_list>(this->_list)) {
               auto& list = std::get<entry_list>(this->_list);
               list.push_back(item);
               return;
            }
            auto  prior = _list_to_entry();
            auto& list  = this->_list.emplace<entry_list>();
            list.push_back(prior);
            list.push_back(item);
         }
      
         template<typename T>
         void _prepend(const T& item) {
            if (std::holds_alternative<std::monostate>(this->_list)) {
               this->_list = item;
               return;
            }
            if (std::holds_alternative<entry_list>(this->_list)) {
               auto& list = std::get<entry_list>(this->_list);
               list.push_front(item);
               return;
            }
            auto  prior = _list_to_entry();
            auto& list  = this->_list.emplace<entry_list>();
            list.push_back(item);
            list.push_back(prior);
         }
      
      public:
         // No-op if the to-be-inserted string is empty.
         void append(const std::string&);
         void append(std::string_view);
         void append(const char* v) {
            return append(std::string_view(v));
         }
         
         // No-op if the to-be-inserted string is empty.
         void prepend(const std::string&);
         void prepend(std::string_view);
         void prepend(const char* v) {
            return prepend(std::string_view(v));
         }
         
         // true if there are no strings to insert
         constexpr const bool empty() const noexcept {
            return std::holds_alternative<std::monostate>(this->_list);
         }
         
         // number of strings to concatenate
         size_t count() const;
         
         // size of the result string
         size_t size() const;
         
         // remove N-th string. throws std::out_of_range on failure
         void remove(size_t n);
         
         std::string build() const;
         std::string build_and_clear();
   };
}