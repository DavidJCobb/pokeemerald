#pragma once

namespace gcc_wrappers::environment::c {
   enum class dialect {
      c89,
      c89_amendment_1,
      c94 = c89_amendment_1,
      c95 = c89_amendment_1,
      c99,
      c11,
      c23,
   };
   constexpr bool operator<(dialect a, dialect b) {
      return (int)a < (int)b;
   }
   constexpr bool operator>(dialect a, dialect b) {
      return (int)a > (int)b;
   }
   
   // Indicates the version of the C standard that GCC has been asked 
   // to compile (i.e. based on the `std` command line argument).
   //
   // We do not distinguish between C23 and "C2X," the name GCC used 
   // on the command line and internally before C23 was finalized. 
   // In general, you should check the compile-time constants we 
   // provide for things like `constexpr` support, before calling 
   // APIs which may require them.
   //
   // If we are not currently loaded into the C compiler, then the 
   // behavior of this function is undefined.
   extern dialect current_dialect();
}