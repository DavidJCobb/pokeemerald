
# `gw::decl::field`

Wrapper for `FIELD_DECL` nodes. These nodes represent non-static data members in a struct.

## Member functions

### General functions

#### `is_class_vtbl_pointer`

Returns true if this field is a polymorphic (i.e. `virtual`) class's hidden v-table pointer.

#### `is_non_addressable`

Returns true if it's illegal to take the address of (or a reference to) this field, e.g. because it's a bitfield. Wraps `DECL_NONADDRESSABLE_P`.

#### `is_padding`

Returns true if this field is structure padding.

#### `offset_in_bytes`

Returns the field's offset in bytes. If the field is a bitfield, then this is the offset of its containing unit (i.e. byte).

#### `offset_in_bits`

Returns the field's offset in bits.

#### `pretty_print`

Returns a string consisting of the field's pretty-printed fully-qualified name.

#### `size_in_bits`

Returns the field's size in bits, accounting for the possibility that the field may be a bitfield.

### Functions for working with bitfields

#### `is_bitfield`

Returns `true` if the field is a bitfield, or false otherwise.

#### `bitfield_offset_within_unit`

If the field is a bitfield, this returns its offset within its containing unit &mdash; that is, the number of bits by which the containing unit must be shifted left in order to access the bitfield's value (presuming you then AND the shifted unit appropriately).

If the field is not a bitfield, then this function returns 0;

#### `bitfield_type`

If the field is a bitfield, this returns the variant type created for it. Otherwise, this function returns empty.