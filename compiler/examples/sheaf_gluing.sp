/* sheaf_gluing.sp — Local-to-global semantic consistency.
 *
 * Spherepop bubbles form a sheaf structure: local admissibility regions
 * whose consistent overlap produces global admissibility. This example
 * demonstrates coherence between nested contexts. See §D.
 */

fn local_computation(x) {
    let patch = bubble {
        x * x + 2 * x + 1
    };
    patch
}

let results = bubble {
    let a = local_computation(3);   /* 16 = (3+1)^2 */
    let b = local_computation(4);   /* 25 = (4+1)^2 */
    let c = local_computation(5);   /* 36 = (5+1)^2 */
    print(a);
    print(b);
    print(c);
    a + b + c
};

print(results);   /* 77 */
