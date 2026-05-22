/* test_bubbles.sp — Bubble nesting and scope isolation */
let outer = bubble {
    let x = 100;
    let inner = bubble { x + 50 };
    inner
};
if (outer != 150) { print("FAIL: nested bubble"); }

let a = bubble { bubble { bubble { 7 } } };
if (a != 7) { print("FAIL: deep nesting"); }

print("bubbles: ok");
