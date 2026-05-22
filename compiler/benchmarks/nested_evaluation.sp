/* nested_evaluation.sp — Benchmark: deeply nested bubble evaluation. */
fn nest(n) {
    if n <= 0 { return 0; }
    bubble { nest(n - 1) + 1 }
}
print(nest(50));
