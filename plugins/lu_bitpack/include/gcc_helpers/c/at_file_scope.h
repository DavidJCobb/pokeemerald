#pragma once

namespace gcc_helpers::c {
   // Check if the parser is currently at file scope. Several GCC functions 
   // act on the current scope rather than letting you choose a scope.
   extern bool at_file_scope();
}