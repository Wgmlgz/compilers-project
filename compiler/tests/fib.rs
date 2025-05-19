let n = 10; // Количество чисел
let a = 0;
let b = 1;

print!("Fibonacci sequence:\n");
for let i = 0; i < n; i += 1 {
    print!(a);
    let next = a + b;
    a = b;
    b = next;
}