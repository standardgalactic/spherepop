/* math.sp — Mathematical utilities for Spherepop. */

fn max(a, b) { if (a > b) { return a; } return b; }
fn min(a, b) { if (a < b) { return a; } return b; }
fn clamp(x, lo, hi) { return max(lo, min(x, hi)); }

fn pow2(n) { return 2 ^ n; }

fn gcd(a, b) {
    while (b != 0) {
        let t = b;
        b = a % b;
        a = t;
    }
    return a;
}

fn lcm(a, b) { return (a * b) / gcd(a, b); }

fn is_prime(n) {
    if (n < 2) { return false; }
    if (n == 2) { return true; }
    let i = 2;
    while (i * i <= n) {
        if (n % i == 0) { return false; }
        i = i + 1;
    }
    return true;
}
