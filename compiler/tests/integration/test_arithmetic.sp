/* test_arithmetic.sp */
let a = 2 + 3;
let b = 10 - 4;
let c = 3 * 7;
let d = 15 / 3;
let e = 2 ^ 8;
if (a != 5) { print("FAIL: addition"); }
if (b != 6) { print("FAIL: subtraction"); }
if (c != 21) { print("FAIL: multiplication"); }
if (d != 5) { print("FAIL: division"); }
if (e != 256.0) { print("FAIL: exponentiation"); }
print("arithmetic: ok");
