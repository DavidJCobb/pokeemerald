
# `gw::constant::integer`

Wrapper for `INTEGER_CST` nodes.

## Member functions

### `sign`

Returns -1, 0, or 1 depending on how the constant's value compares with zero.

### `try_value_signed`

Attempts to retrieve the constant's value, as a `HOST_WIDE_INT`. Fails if the value isn't representable in that type.

### `try_value_unsigned`

Attempts to retrieve the constant's value, as an unsigned `HOST_WIDE_INT`. Fails if the value isn't representable in that type.

### `value<T>`

Attempts to retrieve the constant's value, as an `T`. Fails if the value isn't representable in that type, or if the value isn't representable as a host wide integer (signedness depending on the signedness of `T`).

### `value_type`

Returns the constant's type, as a `gw::type::integral` node.