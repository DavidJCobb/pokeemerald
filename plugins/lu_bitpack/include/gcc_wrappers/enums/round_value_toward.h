#pragma once

namespace gcc_wrappers {
   enum class round_value_toward {
      negative_infinity = -1, // FLOOR_DIV_EXPR
      zero,                   // TRUNC_DIV_EXPR
      nearest_integer,        // ROUND_DIV_EXPR
      positive_infinity,      // CEIL_DIV_EXPR
   };
}