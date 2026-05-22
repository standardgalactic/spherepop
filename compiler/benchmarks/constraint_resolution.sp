/* constraint_resolution.sp — Benchmark: constraint propagation overhead. */
fn chain(n) {
    if n <= 0 { return action(); }
    bubble { chain(n - 1) }
}
print(chain(100));
