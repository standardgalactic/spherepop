/* provenance.sp — Provenance and history inspection. See Monograph §1.3. */
let a = bubble { 3 + 4 };
let b = bubble { 2 + 5 };
print(a);    /* 7 */
print(b);    /* 7 — same terminal value, different provenance */
let h = history();
print(h);
