
# `scope`

This class can wrap any `tree` that is capable of being the scope (i.e. `DECL_CONTEXT`) of some declaration. This includes both types and declarations, as well as things that are probably only considered "declarations" for the sake of uniform behavior within GCC (e.g. `TRANSLATION_UNIT_DECL`).

## Notes

* C has fewer options for scoping than C++, and it doesn't even have an operator for referencing things relative to a named scope (i.e. `::`). That said, there are still some places where things can be scoped to a type; for example, the scope of an enumeration member (`CONST_DECL`) is the enclosing enumeration type (`ENUMERAL_TYPE`), even though the member can be accessed by its unqualified name from outside of that type.
