// Factorial calculation in ArrowRust

let n = 10;
let mut result = 1;
let mut i = 1;

while i <= n {
    result *= i;
    i += 1;
}

print!("Huh? {}", result);
