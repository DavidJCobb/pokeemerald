#pragma once
#include <tuple>

namespace lu::tuples {
   namespace impl {
      template<typename Tuple, typename NewElement>
      struct _prepend;
      
      template<typename NewElement, typename... SubjectElementTypes>
      struct _prepend<std::tuple<SubjectElementTypes...>, NewElement> {
         using type = std::tuple<NewElement, SubjectElementTypes...>;
      };
   }

   template<typename Subject, typename ElementToPrepend>
   using prepend = impl::_prepend<Subject, ElementToPrepend>::type;
}