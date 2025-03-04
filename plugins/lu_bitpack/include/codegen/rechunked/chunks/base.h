#pragma once
#include <cstddef> // size_t
#include <type_traits>
#include <utility>

namespace codegen::rechunked::chunks {
   enum class type {
      array_slice,
      condition,
      padding,
      qualified_decl,
      transform,
   };
   
   class base {
      public:
         virtual ~base() {}
         virtual type get_type() const noexcept = 0;
         virtual bool compare(const base&) const noexcept = 0;
         
      public:
         template<typename Subclass> requires std::is_base_of_v<base, Subclass>
         const Subclass* as() const noexcept {
            if (this->get_type() == Subclass::chunk_type)
               return (const Subclass*) this;
            return nullptr;
         }
         
         template<typename Subclass> requires std::is_base_of_v<base, Subclass>
         Subclass* as() noexcept {
            return const_cast<Subclass*>(std::as_const(*this).as<Subclass>());
         }
   };
}