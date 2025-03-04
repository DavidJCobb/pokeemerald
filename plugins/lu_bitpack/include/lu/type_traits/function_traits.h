#pragma once
#include <functional>
#include <type_traits>

namespace lu {
   namespace impl::function_traits {
      template<typename T> concept has_call_operator_overload = requires {
         { &T::operator() };
      };
      template<typename T> struct is_not_lambda {
         static constexpr bool value = false;
      };
      template<typename R, typename ...Args> struct is_not_lambda<std::function<R(Args...)>> {
         static constexpr bool value = true;
      };
      template<typename R, typename ...Args> struct is_not_lambda<R(*)(Args...)> {
         static constexpr bool value = true;
      };
      template<typename T, typename R, typename ...Args> struct is_not_lambda<R(T::*)(Args...)> {
         static constexpr bool value = true;
      };
      template<typename T, typename R, typename ...Args> struct is_not_lambda<R(T::*)(Args...) const> {
         static constexpr bool value = true;
      };
   }

   // Templates to inspect the value of any non-overloaded function.
   //
   // When using function_traits, the template argument must be decltype(&function), while when using the 
   // individual-trait templates, the template argument must be &function. IntelliSense will accept a 
   // direct reference to the function, but MSVC itself will likely encounter an internal compiler error 
   // that it does not know how to report.

   template<typename T> struct function_traits;
   template<typename R, typename ...Args> struct function_traits<std::function<R(Args...)>> {
      static constexpr const size_t arg_count = sizeof...(Args);
      static constexpr bool is_const_function = false;
      using return_type  = R;
      using context_type = void;

      using arg_tuple = std::tuple<Args...>;
      template<size_t i> using arg_type = typename std::tuple_element<i, std::tuple<Args...>>::type;
   };
   template<typename R, typename ...Args> struct function_traits<R(*)(Args...)> {
      static constexpr const size_t arg_count = sizeof...(Args);
      static constexpr bool is_const_function = false;
      using return_type  = R;
      using context_type = void;

      using arg_tuple = std::tuple<Args...>;
      template<size_t i> using arg_type = typename std::tuple_element<i, std::tuple<Args...>>::type;
   };
   template<typename T, typename R, typename ...Args> struct function_traits<R(T::*)(Args...)> {
      static constexpr const size_t arg_count = sizeof...(Args);
      static constexpr bool is_const_function = false;
      using return_type  = R;
      using context_type = T;

      using arg_tuple = std::tuple<Args...>;
      template<size_t i> using arg_type = typename std::tuple_element<i, std::tuple<Args...>>::type;
   };
   template<typename T, typename R, typename ...Args> struct function_traits<R(T::*)(Args...) const> {
      static constexpr const size_t arg_count = sizeof...(Args);
      static constexpr bool is_const_function = true;
      using return_type  = R;
      using context_type = T;

      using arg_tuple = std::tuple<Args...>;
      template<size_t i> using arg_type = typename std::tuple_element<i, std::tuple<Args...>>::type;
   };

   template<typename Lambda> requires (impl::function_traits::has_call_operator_overload<Lambda> && !impl::function_traits::is_not_lambda<Lambda>::value)
   struct function_traits<Lambda> : public function_traits<decltype(&Lambda::operator())> {};

   template<auto func, size_t n> using type_of_nth_argument = function_traits<decltype(func)>::template arg_type<n>;
   template<auto func> using return_type_of = function_traits<decltype(func)>::return_type;
   template<auto func> using member_function_context_type = function_traits<decltype(func)>::context_type;
   template<auto func> static constexpr bool is_const_function = function_traits<decltype(func)>::is_const_function;
}
