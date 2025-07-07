# Manu Transpiler

**Manu** is a simple experimental transpiler that converts code written in a custom C-like scripting language into x86-64 assembly (NASM syntax) for Linux. It's designed as a learning project to explore concepts of lexical analysis, parsing, abstract syntax tree (AST) generation, and code generation.

The language is dynamically typed, focusing on integer values and basic string manipulation.

## Language Features

The language supports a small set of features:

*   **Variable Declarations:**
    Variables are declared using `var` keyword. While the language is typeless at declaration, the transpiler currently treats most values as 64-bit integers or pointers to strings. The `[]` syntax after a variable name was originally intended for specifying size but is not fully utilized in type checking by the current transpiler version; it's parsed but not strictly enforced in codegen beyond being part of the name.
    ```manu
    var x[] = 123;
    var message[] = "Hello, Manu!";
    var y[] = x + 7;
    ```

*   **ASCII Literals:**
    Numeric values followed by `a` are treated as ASCII character codes.
    ```manu
    var newline_char[] = 10a; // Represents the newline character (ASCII 10)
    ```

*   **String Literals & Concatenation (Conceptual):**
    String literals are enclosed in double quotes. The `+` operator is shown for concatenation with ASCII literals in examples, but full string concatenation is not yet implemented in the code generator.
    ```manu
    var greeting[] = "Hello" + 32a + "World"; // 32a is space. (Conceptual, codegen for + on strings is basic)
    ```

*   **Function Declarations:**
    Functions are declared using the `func` keyword.
    ```manu
    func add(a, b) {
        return a + b;
    }

    func greet(name_str) {
        println(name_str); // Note: println only supports integer printing currently
    }
    ```

*   **Return Statements:**
    Use `return` to return a value from a function.
    ```manu
    func get_number() {
        return 42;
    }
    ```

*   **Expressions:**
    Supports basic arithmetic (`+`, `-`, `*`, `/`, `%`) and comparison (`==`, `!=`, `<`, `>`, `<=`, `>=`) operators.
    ```manu
    var result[] = (5 + 3) * 2;
    var is_equal[] = (result == 16);
    ```

*   **Built-in `println` function:**
    A simple `println` function is provided to print a 64-bit integer value to the console, followed by a newline.
    ```manu
    var my_val[] = 100;
    println(my_val); // Output: 100
    println(my_val + 5); // Output: 105
    ```
    *Note: `println` currently only supports printing integer values. String printing requires further implementation.*

*   **Control Flow (Basic):**
    *   `while` loops:
        ```manu
        var i[] = 0;
        while (i < 5) {
            println(i);
            i = i + 1;
        }
        ```
    *   `for` loops (C-style, comma-separated clauses):
        ```manu
        for (var j[] = 0, j < 3, j = j + 1) {
            println(j);
        }
        ```

*   **Comments:**
    Single-line comments are supported using `//`.
    ```manu
    // This is a comment
    var c[] = 1; // This is an inline comment
    ```

## Building the Transpiler

You'll need `gcc` (or any C99 compatible compiler) and `make` (optional, but convenient).

1.  **Compile the Transpiler:**
    Open your terminal in the project root directory.
    ```bash
    gcc -o manu_transpiler main.c lexer.c parser.c ast.c codegen.c -std=c99 -g
    ```
    Alternatively, if a Makefile is provided in the future:
    ```bash
    make
    ```

## Running the Transpiler

1.  **Create a source file:**
    Write your code in a file with a `.manu` extension (e.g., `test.manu`).
    ```manu
    // test.manu
    var x[] = 10;
    x = x + 5;
    println(x); // Expected output: 15

    func myfunc(a, b) {
        return a * b;
    }

    var result[] = myfunc(3, 7);
    println(result); // Expected output: 21
    ```

2.  **Transpile to Assembly:**
    ```bash
    ./manu_transpiler test.manu
    ```
    This will generate an `output.asm` file in the same directory.

## Assembling and Linking the Output (Linux x86_64)

You'll need `nasm` (Netwide Assembler) and `ld` (linker).

1.  **Assemble:**
    ```bash
    nasm -f elf64 output.asm -o output.o
    ```

2.  **Link:**
    ```bash
    ld output.o -o output_executable
    ```

3.  **Run your program:**
    ```bash
    ./output_executable
    ```
    For the `test.manu` example above, the output would be:
    ```
    15
    21
    ```

## Known Limitations & Future Work

*   **Limited Type System:** Primarily handles 64-bit integers. String support is very basic (literal definition, no runtime manipulation like concatenation yet, `println` does not support strings).
*   **Error Handling:** Parser error reporting is basic. Code generator has minimal error checking.
*   **Assembly Output:** Generated assembly is simple and not highly optimized. Relies on RIP-relative addressing for globals.
*   **Standard Library:** Only `println` (for integers) is available. No file I/O, complex math, or string manipulation functions.
*   **Scoping:** Only global and function scopes are implemented. No block-level lexical scoping for variables yet.
*   **Memory Management for Language:** No garbage collection or explicit memory management for language-level objects (relevant if heaps were used for strings/objects).
*   **Array/Indexing:** Array syntax `x[]` is parsed but full array support (defining size, bounds checking, multi-dimensional arrays) is not implemented. Indexing is basic.
*   **Import System:** `import` statements are parsed but not implemented in the code generator (no module loading/linking).

This project is a work in progress. Future development could focus on addressing these limitations, adding more language features, and improving the robustness of the transpiler.