#pragma once

namespace gcc_wrappers::environment::c_family {
   enum class language {
      none,
      c,
      cpp,
      objective_c,
      objective_cpp,
   };
   
   extern language current_language();
   
   extern bool language_is_cpp_ish(); // C++ or Objective-C++
   extern bool language_is_objective(); // Objective-C(++)
}