/* clio_projection.sp — CLIO projection as stabilized compression. See §E.2. */
fn compress(x) {
    return bubble {
        let sq = x * x;
        let root = sqrt(sq);
        root
    };
}
print(compress(9));    /* 9 */
print(compress(16));   /* 16 */
print(compress(25));   /* 25 */
