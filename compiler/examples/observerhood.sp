/* observerhood.sp — Observer as a semantic operation. See §E. */
let b = bubble { let x = 42; x * 2 };
print(b);  /* 84 */
let h = history();
print(h);
