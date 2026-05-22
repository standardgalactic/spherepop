/* semantic_navigation.sp — Refusal as a recorded trajectory event. See §3.3. */
fn safe_divide(a, b) {
    if (b == 0) {
        refuse();
        return 0;
    }
    return a / b;
}
print(safe_divide(10, 2));   /* 5 */
print(safe_divide(7, 0));    /* 0, refusal recorded */
print(safe_divide(9, 3));    /* 3 */
