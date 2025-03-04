#pragma once
#include <tuple>

namespace lu::tuples {
   namespace impl {
      template<typename Tuple, template<typename> typename Transform>
      struct _transform_types;
      
      template<template<typename> typename Transform, typename... TupleTypes>
      struct _transform_types<std::tuple<TupleTypes...>, Transform> {
         using type = std::tuple<typename Transform<TupleTypes>::type...>;
      };
   }

   // Given a std::tuple<A, B> and a transform struct, transform it into some std::tuple<C, D> where C and D are the result 
   // of running A and B through the transform. The transform should be defined as:
   //
   // template<typename T> struct my_transform { using type = some_permutation_of<T>; };
   //
   template<
      typename Tuple,
      template<typename T> typename Transform
   >
   using transform_types = impl::_transform_types<Tuple, Transform>::type;
}