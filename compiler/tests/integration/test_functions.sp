/* test_functions.sp */
fn add(a, b) { return a + b; }
fn square(x) { return x * x; }
fn fact(n) { if (n <= 1) { return 1; } return n * fact(n - 1); }

if (add(3, 4) != 7) { print("FAIL: add"); }
if (square(9) != 81) { print("FAIL: square"); }
if (fact(5) != 120) { print("FAIL: factorial"); }
print("functions: ok");
