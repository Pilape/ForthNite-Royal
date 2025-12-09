# Monkedore-64-programming-language

This is a compiled stack oriented lagnuage for the MONKEDORE-64 fantasy console.
The syntax is very similar to Forth.

## How to use
If you're on linux you can probably use the executable I accidentally left in the repo.
If not, you gotta compile it yourself.

build.sh probably works if you have clang as your compiler. If you use gcc, use this command instead:
```
gcc -o compiler src/*.c
```
The compiler only uses the standard library so don't worry about dependencies :)
