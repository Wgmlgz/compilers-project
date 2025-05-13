// Factorial calculation in ArrowRust

fn factorial(n: i32) -> i32 {
    let mut result = 1;
    let mut i = 1;
    
    while i <= n {
        result *= i;
        i += 1;
    }
    
    result
}

// Test with different inputs
let a = factorial(5);
print!("Factorial of 5 is: {}", a);

let b = factorial(0);
print!("Factorial of 0 is: {}", b);

let c = factorial(10);
print!("Factorial of 10 is: {}", c);

// Interactive version
print!("Calculating factorials");

let mut n = 1;
while n <= 10 {
    let result = factorial(n);
    print!("{}! = {}", n, result);
    n += 1;
} 