#pragma once
#include "gcc_wrappers/type/untagged_union.h"

namespace bitpacking {
   //
   // Verify that all of the union's members meet the following requirements:
   //
   //  - No two members have the same `lu_bitpack_union_member_id`.
   //
   //  - All members either have `lu_bitpack_union_member_id` or `lu_bitpack_omit`.
   //
   //  - If a member is omitted and has `lu_bitpack_default_value`, it also 
   //    has `lu_bitpack_union_member_id`.
   //
   // Returns false if any requirements are violated.
   //
   extern bool verify_union_members(gcc_wrappers::type::untagged_union type, bool silent = false);
}