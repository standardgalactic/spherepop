/* arithmetic.sp — Demonstrates nested bubble evaluation.
 *
 * In standard notation: 1 + 3 * 2^2 requires operator precedence
 * conventions. In Spherepop, nesting is the structure: the innermost
 * bubble resolves first. See Monograph §1.1.
 */

/* Explicit bubble nesting mirrors evaluation order */
let result = bubble {
    let exp_result = bubble { 2 ^ 2 };
    let mul_result = bubble { 3 * exp_result };
    1 + mul_result
};

print(result);  /* 13 */

/* Verify with direct expression */
let direct = 1 + 3 * 2^2;
print(direct);  /* 13 */
