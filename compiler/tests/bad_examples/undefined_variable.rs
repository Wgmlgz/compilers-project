// Undefined variable errors
let a = 5;
let b = a + c;  // Error: c is not defined

fn test() -> i32 {
    let local = 10;
    return local;
}

// Using a variable out of scope
print!("Value of local: {}", local);  // Error: local is not in scope

// Shadowing and scope errors
{
    let x = 10;
}
print!("X value: {}", x);  // Error: x is not in scope 