// Type mismatch errors
let x: i32 = "this is not an integer";  // Type mismatch: string cannot be assigned to i32

fn add(a: i32, b: i32) -> i32 {
    a + b
}

let result = add("string", 5);  // Type mismatch: function expects i32, got string

// Operation between incompatible types
let y = "hello" + 5;  // Type mismatch: cannot add string and i32

// Boolean operation with non-boolean operands
let z = 10 && 20;  // Type mismatch: logical AND requires boolean operands 