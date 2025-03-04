
# `gw::type::enumeration`

Wrapper for `ENUMERAL_TYPE` nodes.

## Member functions

### `all_enum_members`

Returns a `std::vector` of data structures describing the enum's members &mdash; each pairing a `std::string_view` member name with the member's value (clipped to the range of an `intmax_t`).

### `for_each_enum_member`

Template function which takes a functor as an argument. For each enum member, we construct the same kind of struct as is returned by `all_enum_members` and feed it to the functor as an argument. If the functor returns a `bool`, then we check whether the return value was false and if so, we stop iteration early.

### `has_explicit_underlying_type`

In versions of GCC that support C enums having an explicit underlying type, this checks whether this enum does. In older versions of GCC, this always returns `false`.

### `is_opaque`

Checks if this enum is opaque (i.e. forward-declared, such that its members are not yet known).

### `is_scoped`

Checks if this is a scoped enum (i.e. `enum class` in C++; impossible in C).

### `is_still_being_defined`

This will be `true` if the enum is still being defined &mdash; simply put, if GCC is still parsing the content between the curly braces that wrap the enum's contents. Exactly which properties of the enum are available will depend on GCC's implementation, but as of GCC 14.2.0, everything except for the following is believed to work:

* An enum with no explicitly set underlying type will not *have* an underlying type, because the type is to be computed from the enum's members. Calling the `underlying_type` accessor under these conditions is undefined behavior.

### `underlying_type`

Returns the enum's underlying type. If no underlying type was explicitly specified, or if we're compiled for a version of GCC that predates support for changing a C enum's underlying type, then we return `int_type_node` (wrapped, of course).