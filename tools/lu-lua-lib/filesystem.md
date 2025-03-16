
# `filesystem`

A Lua library which loosely mimics the design of C++'s `std::filesystem` library. This has a dependency on the `classes` library.

## Concepts

### Current working directory

This is initially the script's current working directory. You can call `filesystem.current_path` with an argument to set the current working directory *as seen by the `filesystem` library only*. (Lua doesn't have a built-in way to adjust the CWD script-wide.)

## Classes

### `filesystem.path`

The constructor accepts two optional arguments: the path to use, and an option to normalize the path.

#### Static members

##### `preferred_separator`

The preferred directory separator character.

#### Member functions

##### `path:clone()`

Returns a new `path` instance identical to the current one.

##### `path:append(src)`

Appends `src`, which may be a `path` instance or something from which a `path` may be constructed, to the current path.

* If `src` is an absolute path, or if it's on a different drive, then the contents of the current path are replaced with those of `src`.

##### `path:clear()`

Sets the current instance to an empty path.

##### `path:empty()`

Returns `true` if the current instance is empty, or `false` otherwise.

##### `path:extension()`

Returns the current path's file extension (including the dot) as a string.

* If a filename starts with a dot, the leading dot is never considered the start of a file extension.
* The filenames `.` and `..` return an empty extension.

##### `path:filename()`

Returns the last path segment as a string.

* The filename of `C:/foo.txt` is `"foo.txt"`.
* The filename of `C:/bar` is `"bar"`, even if `bar` is actually a directory.
* The filename of `C:/bar/` is `""`.

##### `path:has_filename()`

Returns `true` if a call to `self:filename()` would return a non-empty string, or `false` otherwise.

##### `path:has_root_directory()`

##### `path:has_root_name()`

##### `path:is_absolute()`

##### `path:is_relative()`

##### `path:lexically_normal()`

Returns a new `path` instance whose content is the content of this instance, but lexically normalized.

* Redundant directory separators are coalesced into a single separator.
* Directory separators are converted to the preferred separator for the current OS (e.g. `\` on Windows).
* The filenames `.` and `..` are removed from the path. The latter removes the parent directory as well: `a/b/../c` normalizes to `a/c`.
* Blank paths normalize to `.`.

##### `path:lexically_relative(base)`

Returns a new `path` instance which is relative to `base` (a `path` instance) and refers to the same location as this instance.

* Returns an empty path if `self` and `base` are on separate roots (e.g. different drives).
* Returns an empty path if `self:is_absolute() ~= base:is_absolute()`.

##### `path:parent_path()`

Returns a new `path` instance whose content is equal to this instance minus one path segment (unless there are none).

* The parent path of `C:/foo/bar` is `C:/foo`.
* The parent path of `C:/foo/bar/` is `C:/foo/bar`.

##### `path:relative_path()`

Returns a new `path` instance whose content is almost identical to this instance, save for the absence of a root.

* The relative path of `C:/foo/bar` is `foo/bar`.

##### `path:stem()`

Returns the current path's filename minus the extension.

* The stem of `foo.txt` is `foo`.
* The stem of `bar` is `bar`.
* The stem of `hello.world.txt` is `hello.world`.
* The stem of `.gitignore` is `.gitignore`.


## Top-level content

### `filesystem.absolute(path)`

Given a relative path, returns an absolute path.

Specifically: given a `path` instance (or something from which a `path` may be constructed), this function returns a new `path` instance; this new path is absolute and is the equivalent of the input path. If the input path is empty, this returns an empty path.

### `filesystem.canonical(path)`

Given a path, returns the canonical path, following any symlinks. If the path doesn't exist, this function throws an error.

### `filesystem.copy_file(src, dst)`

Given a source path and a destination path, attempts to copy a file from the former to the latter. Returns a boolean indicating whether the operation succeeded.

If the paths are relative, then they're treated as being relative to the `filesystem` library's current working directory.

### `filesystem.create_directory(path)`

Given a path, attempts to create a directory at that path. If any intermediate directories don't exiset, they're also created. (For example, if you request the creation of `foo/bar/baz` and `bar` doesn't exist, it and `baz` will be created.) Returns a boolean indicating whether the operation succeeded.

### `filesystem.current_path()`

If called without an argument, this returns a new `path` instance representing the current working directory.

If called with an argument, this adjusts the current working directory *within the `filesystem` library only.*

### `filesystem.file_size(path)`

Returns the size of the file at the given path, if such a file exists and can be opened for reading. Throws an error otherwise.

### `filesystem.open(path)`

Opens a file, returning the same return value as `io.open`. Takes as arguments a `path` instance (or something from which a `path` may be constructed) and an optional string indicating the mode (same as `io.open`).

### `filesystem.remove(path)`

Wraps `os.remove` but takes a `path` instance as an argument.


## Notes

* We generally don't validate paths to filter out OS-specific forbidden characters. However, if a script running on Windows places a `%` glyph into a path and then attempts to execute certain functions on the path, such as `filesystem.copy_file`, an error will occur. This is because the Windows command line uses `%` as the delimiter for variable expansion *and* makes it impossible to escape the `%` symbol within a double-quoted string. Literally impossible. Can't do it. The only way to escape `%` is by doubling it, but that explicitly only works outside of quoted strings.
