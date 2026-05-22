/* visual_topology.sp — Topology via nested containment. See §23–24. */
fn nesting_depth(n) {
    if (n <= 0) { return 0; }
    return 1 + nesting_depth(n - 1);
}
print(nesting_depth(0));   /* 0 */
print(nesting_depth(3));   /* 3 */
print(nesting_depth(7));   /* 7 */
