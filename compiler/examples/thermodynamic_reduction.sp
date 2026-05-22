/* thermodynamic_reduction.sp — Entropy and constraint minimization.
 *
 * The monograph frames admissibility as an accessibility volume in
 * state space. Computations that minimize action are analogous to
 * thermodynamic processes that select minimum-dissipation trajectories.
 * See §C, §12.
 */

fn entropy_estimate(n) {
    /* Approximate semantic entropy: S ≈ n * log(n+1) */
    let log_approx = bubble {
        /* Simple log approximation via series */
        let x = n;
        let sum = 0;
        let term = (x - 1) / x;
        sum + term
    };
    n * log_approx
}

print(entropy_estimate(4));
print(entropy_estimate(8));
print(entropy_estimate(16));
