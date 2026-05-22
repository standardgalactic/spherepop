/* recursive_collapse.sp — Recursive functions with bubble scope. */

fn factorial(n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

print(factorial(5));   /* 120 */
print(factorial(10));  /* 3628800 */

fn fib(n) {
    if (n <= 1) { return n; }
    return fib(n - 1) + fib(n - 2);
}

print(fib(10));   /* 55 */
