# CPPPP

A preprocessor that uses C++ as macro language.

## Why?

Boredom.

## Building

### Requirements

CPPPP depends on `llvm`, `libclang` and `fmt`. While llvm and libclang need to be installed system-wide fmt can be conveniently pulled using conan.

### Building

Fetch fmt

`conan install . -if build --build=missing`

Build

`cmake . -B build` (pro tip: run this with `-DBUILD_TESTS=ON` to build the unit tests as well.

`cmake --build build`

## Usage

### Command line

By default CPPPP will accept input from `stdin`, output to `stdout` and report errors to `stderr`.

If you want to process a file pass it as positional argument to CPPPP, for example `./cpppp examples/example.h`.

Outputting to a file can be achieved by passing the `--output some/filepath.h` parameter. `-o` is an alias for `--output`.

For debugging purposes the parsing result and generated program can be printed to `stderr` by passing `--print-parsed` or `--print-program` respectively. Pro tip: run `tty` in a second terminal and invoke CPPPP with an additional `2>/path/from/tty` (for example `2>/dev/pts/5`). That'll redirect `stderr` to that second terminal.

### CMake

TBD

### Conan

TBD

## Grammar

Here's a probably incorrect EBNF grammar for CPPPP.

```ebnf
valid_token ::= introducer ( define | include | inline | bracketed | shorthand | expression );

define   ::= "define" whitespace (function | global);
function ::= identifier '(' { any } ')' { any } '{' { any } '}';
global   ::= identifier whitespace+ any { any } newline;

include          ::= "include" whitespace+ ( relative_include | system_include ) newline;
relative_include ::= '"' any { any } '"';
system_include   ::= '<' any { any } '>';

inline    ::= '-' { any } '-' introducer;
bracketed ::= '{' { any } '}';
shorthand ::= '=' any { any } introducer;

expression        ::= (simple_expression | function_call) non_ident;
simple_expression ::= identifier;
function_call     ::= identifier "(" { any } ")";


identifier          ::= identifier_start identifier_continue*;
identifier_continue ::= identifier_start | numeric;
identifier_start    ::= alphanumeric | '_';

any          ::= alphanumeric | introducer | non_ident | '_';
non_ident    ::= symbol | newline | whitespace;
alphanumeric ::= alphabetic | numeric;
alphabetic   ::= 'A' | 'B' | 'C' | 'D' | 'E' | 'F' | 'G' | 'H' | 'I' | 'J' | 'K'
               | 'L' | 'M' | 'N' | 'O' | 'P' | 'Q' | 'R' | 'S' | 'T' | 'U' | 'V'
               | 'W' | 'X' | 'Y' | 'Z'
               | 'a' | 'b' | 'c' | 'd' | 'e' | 'f' | 'g' | 'h' | 'i' | 'j' | 'k'
               | 'l' | 'm' | 'n' | 'o' | 'p' | 'q' | 'r' | 's' | 't' | 'u' | 'v'
               | 'w' | 'x' | 'y' | 'z' ;
numeric      ::= '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9';
symbol       ::= '{' | '}' | '[' | ']' | '#' | '<' | '>' | '%' | ':' | ';'
               | '?' | '*' | '+' | '-' | '/' | '^' | '&' | '|' | '~' | '!'
               | '=' | ',' | '|' | '"' | "'" | '.';
newline      ::= '\n' | '\r';
whitespace   ::= ' ' | '\t' | '\f';

introducer   ::= '@';
```

## Acknowledgements

* [wreien](https://github.com/wreien) for the lovely idea

