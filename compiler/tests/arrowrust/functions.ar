// Function definition and testing

// Function that returns an i32
fn add(a: i32, b: i32) -> i32 {
    a + b  // Implicit return
}

// Function with explicit return
fn max(a: i32, b: i32) -> i32 {
    if a > b {
        return a;
    } else {
        return b;
    }
}

// Function that takes a bool parameter
fn greet(name: str, formal: bool) -> () {
    if formal {
        print!("Hello, {} Sir/Madam.", name);
    } else {
        print!("Hey, {}!", name);
    }
}

// Void function with no parameters
fn say_hello() {
    print!("Hello, World!");
}

// Testing the functions
let x = 10;
let y = 20;
let sum = add(x, y);
print!("Sum of {} and {} is: {}", x, y, sum);

let maximum = max(x, y);
print!("Maximum of {} and {} is: {}", x, y, maximum);

greet("Alice", true);
greet("Bob", false);

say_hello(); 