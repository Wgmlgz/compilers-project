// Function-related errors

// Wrong number of arguments
fn sum(a: i32, b: i32) -> i32 {
    a + b
}

let result1 = sum(1);  // Error: too few arguments
let result2 = sum(1, 2, 3);  // Error: too many arguments

// Return type mismatch
fn get_number() -> i32 {
    "not a number"  // Error: expected i32, got str
}

// Missing return in non-void function
fn always_positive(x: i32) -> i32 {
    if x > 0 {
        x  // Returns x if positive
    }
    // Error: no return in all code paths
}

// Using undefined function
let calc_result = calculate(10);  // Error: function 'calculate' is not defined

// Void function with return value
fn greet() -> () {
    return 5;  // Error: void function returning a value
} 