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
The language is stack oriented, which means you manipulate data using a stack.
To add a number to the stack you write a number. All numbers must be unsigned 16-bit integers.
The language is also case insensitive.

### Reverse polish notation
Like Forth, and unlike most other languages. Expressions are handled in reverse polish notation.
```
1 + 1
```
becomes
```
1 1 +
```
Writing a number pushes it onto the stack. And + adds together the top elements of the stack and pushes the result.

### Primitives
The stacks are read from left to right. With the righmost element being the top of the stack.

<-----------------------------

|  Name  | Stack before | Stack after  |      Other effects      |
|:------:|:-------------|:-------------|:------------------------|
| NOP    |              |              | Does nothing            |
| HALT   |              |              | Stops execution         |
| DUP    | a            | a a          |                         |
| OVER   | a b          | a b a        |                         |
| POP    | a            |              |                         |
| NIP    | a b          | b            |                         |
| SWAP   | a b          | b a          |                         |
| ROT    | a b c        | b c a        |                         |
| LOAD   | a            | ram[a]       |                         |
| STORE  | a b          | ram[b] = a   |                         |
| LOADb  | a            | (byte)ram[a] |                         |
| STOREb | a b          |              | ram[b] = (byte)a        |
| \+     | a b          | a+b          | Sets carry on overflow  |
| \-     | a b          | a-b          | Sets carry on underflow |
| ADDc   | a b          | a+b+carry    | Sets carry on overflow  |
| SUBc   | a b          | a-b-carry    | Sets carry on underflow |
| <<     | a b          | a<<b         |                         |
| \>\>   | a b          | a\>\>b       |                         |
| bNAND  | a b          | ~(a & b))    |                         |
| NAND   | a b          | !(a && b))   |                         |
| =      | a b          | a==b         |                         |
| \>     | a b          | a>b          |                         |
| <      | a b          | a<b          |                         |

### Functions/Words

