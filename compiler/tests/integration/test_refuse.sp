/* test_refuse.sp */
fn safe_div(a, b) {
    if (b == 0) {
        refuse();
        return -1;
    }
    return a / b;
}
let r1 = safe_div(10, 2);
let r2 = safe_div(5, 0);
if (r1 != 5) { print("FAIL: safe_div normal"); }
if (r2 != -1) { print("FAIL: safe_div refused"); }
print("refuse: ok");
