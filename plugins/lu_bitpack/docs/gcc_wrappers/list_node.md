
# `list_node`

A wrapper for `TREE_LIST` nodes. These are ordered linked lists of key/value pairs; for a more typical "list," see `chain`.

## Notes

### The constructor sucks, but trying to make it suck less would have the opposite effect

The `list_node` class is designed primarily around accessing and reading lists that already exist, rather than creating new lists. As a result, the class has to wrap a `TREE_LIST` node that definitely exists; and so when you construct an instance, we have to *create* a `TREE_LIST` node.

Here's the rub: `TREE_LIST` nodes *are* the pairs in a list. When we create a `TREE_LIST` node, we are also creating a key/value pair; it's not possible to have an "empty list" per se. Accessors for potentially-empty (i.e. potentially `NULL_TREE`) lists will return `optional_list_node` for this reason.

The reason we use a `node` subclass and not a view like `chain` is because an empty list is a non-existent list. Say there's a list belonging to some other node (e.g. the list of arguments on a `FUNCTION_TYPE`), and you access an empty list. If this wrapper were a view, able to wrap `NULL_TREE`, then we'd have to define appending and prepending as an option wherein if you append to an empty (i.e. nonexistent) list, we create the list head on demand... but then how do we know what the list head should belong to? We don't and can't know about the `FUNCTION_TYPE`. So in this hypothetical, code like this looks like it should work, but it'd actually fail, at least for a varargs function with no fixed arguments (possible in C23 onward):

```c++
auto args = my_function.arguments();
args.append({}, desired_type_1);
args.append({}, desired_type_2);
```

With the current approach, we instead have:

```c++
auto args = my_function.arguments();
if (args) {
   args->append({}, desired_type_1);
   args->append({}, desired_type_2);
} else {
   args = list_node({}, desired_type_1);
   args.append({}, desired_type_2);
   
   // we don't currently offer a wrapped accessor for this because 
   // i don't know what it'd break, so we have to go under the hood 
   // with raw GCC accessors
   TYPE_ARG_TYPES(my_function.unwrap()) = args.unwrap();
}
```

In an *attempt* to make the constructor *less* of a footgun, it's designed to require an explicitly-passed key and value (i.e. they don't default to null) for the created list-head pair.