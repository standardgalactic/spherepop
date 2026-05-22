/* action_minimization.sp — Action cost and nesting depth. See §12. */
let deep = bubble { bubble { bubble { bubble { 2 + 2 } } } };
let flat = 2 + 2;
print(deep);   /* 4 */
print(flat);   /* 4 */
let h = history();
print(h);
