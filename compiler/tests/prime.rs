let num = 17;
let is_prime = 1; // 1 = true, 0 = false

if num <= 1 {
    is_prime = 0;
} else {
    for let i = 2; i * i <= num; i += 1 {
        if num % i == 0 {
            is_prime = 0;
            break;
        }
    }
}

if is_prime {
    print!("is prime: ");
    print!(num)
} else {
    print!("is not prime: ");
    print!(num)
}