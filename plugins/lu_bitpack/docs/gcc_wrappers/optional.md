
# `optional`

This class is similar to `std::optional`, but for a few things:

* We offer constructors and member functions that handle downcasting of a wrapped node.
* We can be constructed from a raw `tree`, asserting that it is of the correct type.
* Since the underlying representation is a `tree` (or a wrapper thereof), we can use `NULL_TREE` to indicate an empty state, rather than burning several bytes on a `bool` and the padding it'd introduce.

## Maintenance notes

The class currently uses a `union` of a `value_type` and a bare `tree`. This is to avoid ugly pointer casts in `operator->` (i.e. `return (value_type*) this`).

### Requirements for `constexpr`

The use of a `union` internally, in conjunction with C++26 `std::is_within_lifetime`, would (in isolation, ignoring limitations on all associated classes including the `value_type` itself) make this class `constexpr`-friendly, so there exists some checks for `std::is_constant_evaluated()` in its member functions. If GCC's internals ever become more `constexpr`-friendly in the future (which I'll grant seems unlikely), this could potentially come in handy in the future.

* `constexpr` node boilerplate
  * In the macro: `T::is`
  * In the macro: `T::as`
  * In the macro: `T::wrap`
  * In most subclasses: `T::raw_node_is`
* C++26
  * At run-time, the `union` members `_node` and `_tree` are bit-for-bit identical, since the former is just a `tree` wrapper; we can therefore tell whether we have a `node` by checking `_tree != NULL_TREE`. During constant evaluation, however, this isn't allowed; we have to check whether a given union member `std::is_within_lifetime` to know whether it's safe to access. (In other words, `std::is_within_lifetime` (C++26) acts as a union tag, without us having to burn space on one at run-time).