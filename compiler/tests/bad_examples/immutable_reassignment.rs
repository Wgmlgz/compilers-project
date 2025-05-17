// Immutable reassignment errors
let x = 5;
x = 10;  // Error: cannot assign to immutable variable 'x'

let y = "hello";
y += " world";  // Error: cannot modify immutable variable 'y'

// Correct way:
let mut z = 42;
z += 8;  // This is valid

// This is also an error:
let const_val = 100;
const_val *= 2;  // Error: cannot assign to immutable variable 'const_val'

fn modify_param(param: i32) -> i32 {
    param += 1;  // Error: function parameters are immutable by default
    param
} 