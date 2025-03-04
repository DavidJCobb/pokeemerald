
# Requirements for C++ support

As of this writing, I've only developed this plug-in for use when compiling C code with GCC. Several accessors don't support C++, and you should expect run-time link errors when trying to load it while compiling C++ code.

## Architecture

GCC uses separate executables for C versus C++ code. We link to GCC's exported functions at run-time, and so if we directly reference, in source code, an identifier that isn't present at run-time in whatever executable we're linking with, then we'll fail to load.

One potential remedy would be a preprocessor flag, to build the plug-in separately for C or C++.

Alternatively, we could use [`dlsym(RTLD_DEFAULT, "mangled_name_here")`](https://pubs.opengroup.org/onlinepubs/009696899/functions/dlsym.html) to access internal GCC functions that are only present for one language or another. Cast the returned pointer to the appropriate function type. This is basically the Linux equivalent to `GetProcAddress(GetModuleHandleA(NULL), "mangled_name_here")`. We can cache returned functions in a singleton to avoid having to grab them more than once, and potentially optimize further by having the singleton pre-grab everything on startup so we don't have to null-check every time we want to call one of these. I suspect this is more-or-less what the linker does for us when we reference GCC functions directly.

## Existing helpers

`gcc_wrappers::environment::c_family::current_language()` and friends exist to wrap language checks, and could be used for... whatever we decide to do about this someday.

## Places that I know need explicit support for C++

* Anywhere we call `comptypes`, i.e. `gw::type::base::operator==`
* Probably the code to finalize a `gw::decl::function` when setting its root block
* `gw::scope` would need to be updated to support scopes unique to C++
  * Namespaces
* `gw::decl::variable::is_declared_constexpr` and friends, which need to use `DECL_DECLARED_CONSTEXPR_P` for C++

### Structured bindings and related edge-cases

`DECL_DECOMPOSITION_P(node)` is non-zero if `node` is a `VAR_DECL` which represents a decomposition, e.g. `x` and `y` in the example below. In this case, GCC gives `node` a data structure that points to the decomposed entity (`my_foo` in the example below), and sets flags like `DECL_HAS_VALUE_EXPR` on `node` to indicate how to access the decomposed value.

We need to honor `DECL_HAS_VALUE_EXPR_P`. The edgiest edge case here is a structured binding that decomposes a struct containing bitfields, such that the decomposed-to variables act like references to bitfields (something that is ordinarily impossible to create). They aren't, actually; they aren't `REFERENCE_TYPE`s, but rather use `DECL_HAS_VALUE_EXPR_P`.

```c++
struct foo {
   int a : 7;
   int b : 3;
};
foo my_foo;

auto& [x, y] = my_foo;
```

Here, `x` and `y` are `VAR_DECL`s which use `DECL_HAS_VALUE_EXPR_P` to indicate that their values are located at `my_foo.a` and `my_foo.b`. They aren't `REFERENCE_TYPE`s; rather, it's more akin to `#define x my_foo.a` but with proper scoping.

## Individual features we'd need to support

* Checking/altering whether a variable is `thread_local` (investigate `DECL_THREAD_LOCAL_P`)

