
# `gw::type::container`

Wrapper for `RECORD_TYPE` and `UNION_TYPE` nodes.

## Member functions

### `for_each_field`

Runs a functor on each `FIELD_DECL` in the type, with the field passed as a `gw::decl::field` wrapper. If the functor returns `bool`, then we check the return value and if it's `false`, we stop iteration early.

Note that for C++, the functor may receive fields that are not ordinarily referenceable in code (e.g. a polymorphic struct or class's v-table pointer), and may not receive fields that are ordinarily referenceable in code (e.g. members of an anonymous sub-struct). Consider using `for_each_referenceable_field` if you need to access fields using the same "rules" as written code.

### `for_each_referenceable_field`

Runs a functor for each referenceable field in the type, with the field passed as a `gw::decl::field`. A field is referenceable if it's a direct non-static data member with a name, or if it's a referenceable member of an anonymous (unnamed) sub-struct. Basically, this follows the same rules as member access in written code.

If the functor returns `bool`, then we check the return value and if it's `false`, we stop iteration early.

### `for_each_static_data_member`

Runs a functor for each static data member (`VAR_DECL`) in the type, with the member passed as a `gw::decl::variable`. If the functor returns `bool`, then we check the return value and if it's `false`, we stop iteration early.

### `member_chain`

Returns the type's raw member chain, i.e. `TREE_FIELDS(type)`.

### `is_still_being_defined`

This will be `true` if the type is still being defined &mdash; simply put, if GCC is still parsing the content between the curly braces that wrap the type's contents. Exactly which properties of the container type are available will depend on GCC's implementation.