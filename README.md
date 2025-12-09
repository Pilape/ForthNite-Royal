# ForthNite Royal
A compiled stack oriented lagnuage for the MONKEDORE-64 fantasy console.
The syntax is very similar to Forth.

## How to use

### Building
If you're on linux you can probably use the executable I accidentally left in the repo.
If not, you gotta compile it yourself.

build.sh probably works if you have clang as your compiler. If you use gcc, use this command instead:
```
gcc -o compiler src/*.c
```
The compiler only uses the standard library so don't worry about dependencies :)

### Running
The compiler only needs 1 argument which is the path to the file you want to compile. The file extension can be anything you want as long as it's a text file. But the official file extension is .fn.
```
./compiler path/to/file.fn
```
The program outputs a .hex file which is a text file containing the bytecode instructions from the program, and a .rom file which contains the actual program bytecode in binary format.

## The language itself

### Reverse polish notation

### Primitives

### Functions/Words
