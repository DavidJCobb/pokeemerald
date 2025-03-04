#pragma once
#include <type_traits>

namespace lu {
   template<typename T> // CRTP
   class singleton {
      private:
         static T* instance;

      protected:
         singleton() {
            static_assert(std::is_base_of_v<singleton<T>, T>);
            T::instance = (T*) this;
         }

         // hack to work around the inability to access T::T if it's protected, without 
         // requiring T to friend singleton:
         struct _unprotect : public T {
            _unprotect() : T() {}
         };

      public:
         static T& get() {
            static _unprotect instance;
            return instance;
         }
         static T& get_fast() {
            return *T::instance;
         }

         singleton(singleton&&) = delete;
         singleton(const singleton&) = delete;

         singleton& operator=(singleton&&) = delete;
         singleton& operator=(const singleton&) = delete;
   };
   template<typename T> T* singleton<T>::instance = nullptr;
}