// Format string errors

// Too few arguments
let name = "Alice";
print!("Hello, {}, your ID is {}", name);  // Error: missing argument for format specifier

// Too many arguments
let id = 42;
print!("ID: {}", id, "extra");  // Error: too many arguments for format string

// Missing closing brace
let count = 5;
print!("Count: {", count);  // Error: invalid format string, unclosed brace

// Empty braces in format string
print!("Empty braces: {}");  // Error: no value for format specifier

// Nested braces
let nested = 10;
print!("Nested: {{nested}}");  // This is actually valid in Rust (prints "{nested}")

// Format specifier in string literal
let s = "Format: {}";  // This is valid, just a string
print!("{}", s); 