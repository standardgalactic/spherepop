/* manifold_walk.sp — Walking through semantic space via nested bubbles. */
fn walk(n) {
    let mut acc = 0;
    let mut i = 0;
    while (i < n) {
        let step = bubble { i * i };
        acc = acc + step;
        i = i + 1;
    }
    return acc;
}
print(walk(5));   /* 0+1+4+9+16 = 30 */
