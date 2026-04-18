#![cfg_attr(feature = "axstd", no_std)]
#![cfg_attr(feature = "axstd", no_main)]

#[cfg(feature = "axstd")]
use axstd::println;

fn fib(n: u32) -> u32 {
    if n < 2 {
        return n;
    }

    let mut a = 0;
    let mut b = 1;
    let mut i = 2;
    while i <= n {
        let next = a + b;
        a = b;
        b = next;
        i += 1;
    }
    b
}

fn is_prime(n: u32) -> bool {
    if n < 2 {
        return false;
    }

    let mut d = 2;
    while d * d <= n {
        if n % d == 0 {
            return false;
        }
        d += 1;
    }
    true
}

fn print_progress(step: usize, total: usize) {
    let width = 12;
    let filled = step * width / total;
    let mut bar = [b'.'; 12];
    let mut i = 0;
    while i < filled {
        bar[i] = b'#';
        i += 1;
    }

    let bar = core::str::from_utf8(&bar).unwrap_or("????????????");
    println!("  stage {step}/{total} [{bar}]");
}

#[cfg_attr(feature = "axstd", unsafe(no_mangle))]
fn main() {
    println!();
    println!("=== ChenlongOS Mini Demo ===");
    println!("Hello from ArceOS!");
    println!("arch     : {}", option_env!("AX_ARCH").unwrap_or("unknown"));
    println!(
        "platform : {}",
        option_env!("AX_PLATFORM").unwrap_or("unknown")
    );
    println!("log      : {}", option_env!("AX_LOG").unwrap_or("unknown"));

    println!();
    println!("countdown:");
    for t in (1..=3).rev() {
        println!("  T-{t}");
    }
    println!("  lift off!");

    println!();
    println!("fibonacci:");
    for i in 0..8 {
        println!("  fib({i}) = {}", fib(i));
    }

    println!();
    println!("primes below 20:");
    for n in 2..20 {
        if is_prime(n) {
            println!("  {n}");
        }
    }

    println!();
    println!("boot progress:");
    for step in 1..=4 {
        print_progress(step, 4);
    }

    println!();
    println!("demo complete.");
}

