#ifndef GUARD_LU_TEST_HELPERS
#define GUARD_LU_TEST_HELPERS

// Indicates that a field or type should be omitted from serialization.
#define LU_BP_OMIT             __attribute__((lu_bitpack_omit))

// Indicates the default value for a field or type. This is generally only 
// useful for fields that are omitted from bitpacking entirely.
#define LU_BP_DEFAULT(x)       __attribute__((lu_bitpack_default_value(x)))

// Annotate fields or types with tags for stat-tracking purposes. The XML output 
// will tell you how many times the tag appears, and the amount of space devoted 
// to tagged data.
#define LU_BP_CATEGORY(name) __attribute__((lu_bitpack_stat_category(name)))

// Indicates that a field or type should be serialized into the bitstream 
// verbatim, as if by memcpy (albeit with a potentially non-byte-aligned 
// destination).
#define LU_BP_AS_OPAQUE_BUFFER __attribute__((lu_bitpack_as_opaque_buffer))

// Integer values only: indicates the number of bits used to serialize the 
// value. If not specified explicitly, it will be inferred from the type, 
// min/max values, and the bit-widths of bitfields.
#define LU_BP_BITCOUNT(n)      __attribute__((lu_bitpack_bitcount(n)))

// Integer values only: indicates the allowed minimum and maximum values. 
// Used to infer the bitcount whenever that isn't explicitly specified.
#define LU_BP_MINMAX(min, max) __attribute__((lu_bitpack_range(min, max)))

// Indicates that a char array (or nested array thereof) is a string.
#define LU_BP_STRING           __attribute__((lu_bitpack_string))

// Indicates that a char array (or nested array thereof) is a string and 
// must have a terminator byte while in memory.
#define LU_BP_STRING_WT        LU_BP_STRING

// Indicates that a char array (or nested array thereof) is a string and 
// does not require a terminator byte while in memory; the string value 
// will have a terminator byte only if it's shorter than the array's size.
#define LU_BP_STRING_UT        __attribute__((lu_nonstring)) LU_BP_STRING

// Indicates that a value should be stored in an alternate representation 
// within the bitstream, using the specified functions to convert it.
#define LU_BP_TRANSFORM(pre_pack, post_unpack) \
   __attribute__((lu_bitpack_transforms("pre_pack=" #pre_pack ",post_unpack=" #post_unpack)))

#define LU_BP_TRANSFORM_AND_NEVER_SPLIT(pre_pack, post_unpack) \
   __attribute__((lu_bitpack_transforms("pre_pack=" #pre_pack ",post_unpack=" #post_unpack ",never_split_across_sectors")))

// Indicate the value that acts as a union's tag, when that value is not 
// inside of [all of the members of] the union itself. The union must be 
// located somewhere inside of a containing struct (named tag or typedef) 
// and the attribute's argument must be a string denoting the name of a 
// member of said struct that appears before the union does.
#define LU_BP_UNION_TAG(n) __attribute__((lu_bitpack_union_external_tag( #n )))

// Indicate which member-of-member of this union acts as its tag.
// For example, `name` means that `__union.__any_member.name` is the tag.
#define LU_BP_UNION_INTERNAL_TAG(n) __attribute__((lu_bitpack_union_internal_tag( #n )))

// Use on a union member to specify the tag value that indicates that that 
// member is active. Must be an integral value.
#define LU_BP_TAGGED_ID(n) __attribute__((lu_bitpack_union_member_id(n)))

#endif