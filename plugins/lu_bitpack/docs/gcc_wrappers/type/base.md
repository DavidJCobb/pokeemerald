
# `gw::type::base`

A wrapper for any `..._TYPE` tree node.

## Facts about types

### Types can have two names

Unlike C++, the C language doesn't have namespaces. It does, however, have [name spaces](https://en.cppreference.com/w/c/language/name_space). These are predefined and nameless collections of the names that refer to various entities in a program:

* **Labels**
* **Tags:** When enum, struct, or union types are declared with an identifier given after the keyword, that identifier is a <dfn>tag</dfn> and goes in the <dfn>tag name space</dfn>.
* **Members:** Each struct and union type has its own <dfn>member name space</dfn> in which members are looked up.
* **(C23) Global attributes**
* **(C23) Non-standard attributes**
* **Ordinary identifiers**: Everything not in the above categories.

These name spaces don't overlap, so you can do this without any errors:

```c
struct foo {};

void foo(void) {
   int foo = 0;
foo:
   ++foo;
   if (foo < 5)
      goto foo;
}

// bonus:
typedef struct bar {} bar;
// you can now refer to this as `bar` or `struct bar`
```

Names created via the `typedef` keyword are ordinary identifiers. This means that the following code introduces the name `T` to the tag name space (accessible via `identifier_global_tag` within GCC's internals) and the name `U` to the ordinary name space (accessible via `lookup_name` within GCC's internals):

```c
typedef struct T {} U;
```

If you're working with the `..._TYPE` node for `U` (the `typedef` keyword creates a new `..._TYPE` node that basically wraps around the original), you can access these identifiers through different means:

* `gw::type::base::name()` will return the name of the `typedef`, if a type was created by a type, or its tag name otherwise.
* `gw::type::base::tag_name()` will return the type's tag name, if it has one.
* `gw::type::base::declaration()` will return the `gw::decl::type_def` node that created the type, if it was created via `typedef`. If it returns non-null, you can then call `name()` on it.

### When are types created?

Types are created under the following circumstances:

* When a type is declared.
* When a function is declared: every function signature is its own type (`FUNCTION_TYPE`).
* When you use the `typedef` keyword to create a new name for a type, you are actually creating an entirely new `..._TYPE` node that acts as a synonym of the original.
  * Given a type, `TYPE_NAME(t)` will return either the `IDENTIFIER_NODE` describing the type's name, or the `TYPE_DECL` node describing the `typedef` declaration.

