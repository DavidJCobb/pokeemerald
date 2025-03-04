#pragma once

//
// The <plugin-version.h> header doesn't use an include guard or #pragma once, 
// and it also causes unused variable warnings if we only want the preprocessor 
// macros for the current GCC version.
//

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include <plugin-version.h>
#pragma GCC diagnostic pop