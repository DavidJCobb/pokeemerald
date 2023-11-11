
* We are compiling in some weird dialect in between C89 and C99, so variables must be declared before any statements in their containing block. We can still use `//` comments, though.

* As of this writing, the version of GCC installed is 11. I don't know if this is controlled by the WSL installation process or the pokeemerald project configuration and at this point I'm not willing to risk upgrading it

** Side-effect: `#pragma region` isn't supported [yet](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53431) and will cause a compiler error; ditto for `endregion`.

* We use the GNU Assembler (`as`); [documentation here](https://sourceware.org/binutils/docs/as/index.html#SEC_Contents).