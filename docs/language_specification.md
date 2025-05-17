# ArrowRust Language Specification

ArrowRust is a statically-typed programming language inspired by Rust, designed for educational purposes and compiler development. It offers a simplified subset of Rust's features with a focus on readability and ease of learning.

## Overview

ArrowRust combines elements of imperative and functional programming with a C-like syntax. It features:

- Static typing with type inference
- Immutable variables by default with optional mutability
- First-class functions
- Expressions-based semantics
- Simple memory model without manual memory management

## Basic Syntax

### Variables

Variables are declared using the `let` keyword:

```rust
// Immutable variable with type inference
let x = 10;

// Mutable variable
let mut y = 20;

// Explicit type annotation
let z: i32 = 30;
let mut counter: i32 = 0;
```

### Primitive Types

- `i32`: 32-bit signed integers
- `bool`: Boolean values (`true` or `false`)
- `str`: String values
- `()`: Unit type (similar to void)

### Operators

#### Arithmetic Operators
- `+`: Addition
- `-`: Subtraction
- `*`: Multiplication
- `/`: Division
- `%`: Modulo

#### Comparison Operators
- `==`: Equal
- `!=`: Not equal
- `<`: Less than
- `>`: Greater than
- `<=`: Less than or equal
- `>=`: Greater than or equal

#### Logical Operators
- `&&`: Logical AND
- `||`: Logical OR
- `!`: Logical NOT

#### Assignment Operators
- `=`: Assignment
- `+=`: Add and assign
- `-=`: Subtract and assign
- `*=`: Multiply and assign
- `/=`: Divide and assign
- `%=`: Modulo and assign

### Control Flow

#### If Expressions

```rust
if expression {
    // code
} else {
    // code
}
```

If expressions can be used as values:

```rust
let max = if a > b { a } else { b };
```

#### While Loops

```rust
while expression {
    // code
}
```

### Functions

Function declarations use the `fn` keyword:

```rust
fn function_name(param1: Type1, param2: Type2) -> ReturnType {
    // code
    return_expression
}
```

Functions can return a value by ending with an expression without a semicolon:

```rust
fn add(a: i32, b: i32) -> i32 {
    a + b  // Implicit return
}
```

Or with an explicit return statement:

```rust
fn max(a: i32, b: i32) -> i32 {
    if a > b {
        return a;
    } else {
        return b;
    }
}
```

A function with no return value has the return type `()`:

```rust
fn greet(name: str) -> () {
    print!("Hello, {}!", name);
}
```

The return type can be omitted for functions returning `()`:

```rust
fn greet(name: str) {
    print!("Hello, {}!", name);
}
```

### Input/Output

The language provides a built-in `print!` macro for output:

```rust
print!("Hello, World!");
print!("The value is: {}", x);
print!("{} + {} = {}", a, b, a + b);
```

## Examples

### Factorial Calculation

```rust
fn factorial(n: i32) -> i32 {
    let mut result = 1;
    let mut i = 1;
    
    while i <= n {
        result *= i;
        i += 1;
    }
    
    result
}

let n = 5;
let result = factorial(n);
print!("{}! = {}", n, result);
```

### Greeter Function

```rust
fn greet(name: str, formal: bool) -> () {
    if formal {
        print!("Hello, {} Sir/Madam.", name);
    } else {
        print!("Hey, {}!", name);
    }
}

greet("Alice", true);
greet("Bob", false);
```

## Compilation

The ArrowRust compiler translates source code into a custom RISC assembly language that can be executed on a virtual machine. The virtual machine:

- Has 65536 memory cells (32-bit each)
- Features 32 registers (x0-x31)
- Includes a program counter
- Supports basic RISC operations

## Conclusion

ArrowRust is designed to balance simplicity with expressiveness, making it ideal for learning compiler design and programming language concepts. While it lacks advanced features like generics, traits, or advanced memory management, it provides a solid foundation for understanding core programming language principles. 