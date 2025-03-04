
# `gw::decl::variable`

Wrapper for `VAR_DECL` nodes. These nodes represent variables.

Note that a `VAR_DECL` node only indicates the *existence* of a variable declaration. It may have a source location, but this indicates a file and line number, not (as GCC conceives of it) the variable's *placement within code*. As such, when creating local variables, you may also need a `gw::expr::declare`: the expression that declares the variable. You can call `make_declare_expr` on a `gw::decl::variable` to quickly create one of those, and then append it into a function's statement list.

## Member functions

### Boolean accessors

Boolean accessors generally take the form of `is_...`, `make_...`, and `set_is_...`, where the former function is a getter, the latter is a setter, and the "make" function just calls `set_is_...(true)`.

#### `..._declared_constexpr`
Accessors to check whether the variable was declared as `constexpr`.

The setters assert that the current GCC version supports `constexpr` and, if you try to make the variable `constexpr`, that the compiler is currently using a version of the C standard which supports `constexpr` (i.e. C23 onward).

#### `..._defined_elsewhere`

#### `..._externally_accessible`
Accessors for the `TREE_PUBLIC` flag, which indicates that the variable is accessible from outside of the current translation unit.

#### `..._ever_read`
Accessors for the `DECL_READ_P` flag, which indicates whether the variable has ever been used in any manner other than having its value set.

### Other members

#### `commit_to_current_scope`
This can be called while the parser is still running. It will introduce the variable into the current function scope, if the parser is inside a function, or into the file/global scope (but not the extern scope) otherwise. Note that unfortunately, when compiling C, there is no way to introduce the variable into the current *lexical* scope (i.e. the individual code block currently being parsed), as GCC doesn't expose the functions or state needed.

This has the side effect of finalizing compilation of the variable, generating whatever assembler labels, etc., are necessary for it. This also creates a `DECLARE_EXPR` for you, in the parser's current statement list.

This function asserts that the variable is not already in a scope. However, GCC doesn't expose enough of its internals for us to check whether the scope contains a variable with the same name. If a variable with the same name already exists at the scope into which this function would be introduced, GCC will emit a user-visible compiler error, but there's no (good) way for us to detect this (preemptively or after the fact).

#### `initial_value` and `set_initial_value`
Accessors for the variable's initial value, if it has one.

#### `make_declare_expr`
Creates a `gw::expr::declare` representing the variable's location within source code.

#### `make_file_scope_extern`
Attempts to introduce this variable into the file/global and extern scopes, such that name lookups and similar operations can find it. This has the side effect of finalizing compilation of the variable, generating whatever assembler labels, etc., are necessary for it.


## Notes

* GCC internals assume that a variable is global "if its storage is not automatic." In practice, this means that `is_global_var(t)` in `tree.h` returns `(TREE_STATIC(t) || DECL_EXTERNAL(t))`.