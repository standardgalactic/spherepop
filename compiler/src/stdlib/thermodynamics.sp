/* thermodynamics.sp — Entropy and action utilities. See §C, §G. */
fn boltzmann_entropy(n_states) {
    /* S = k_B * ln(W) — approximated here for integer state counts */
    if (n_states <= 0) { return 0; }
    /* Rough natural log: ln(n) ≈ (n-1)/n + (n-1)^2/(2n^2) ... */
    let x = n_states;
    let approx = (x - 1) / x;
    return approx
}

fn action_cost(depth, steps) {
    return depth * steps * 0.1
}
