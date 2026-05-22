/* bubble_nesting.sp — Nested admissibility regions.
 *
 * Each bubble is a topological object: a bounded region with its own
 * scope and history. Inner bubbles are resolved before outer ones.
 * Scope is physical containment, not a call stack. See §2.2.
 */

let outer = bubble {
    let x = 10;
    let inner = bubble {
        let y = 20;
        x + y     /* x is visible: inner is contained within outer */
    };
    inner * 2
};

print(outer);   /* 60 */

/* Demonstrate scope boundary: y is not visible outside inner bubble */
let x = 100;
let b = bubble {
    let y = 200;
    x + y
};
print(b);       /* 300 */
print(x);       /* 100 — outer x unchanged */
