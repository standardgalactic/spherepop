# Operational Semantics

See Monograph §1.2, §3.1–3.5 and Appendix A for the formal development.

## Pop Rule

    B = (U, ∂U, σ, τ) is innermost and locally admissible
    ──────────────────────────────────────────────────────
    eval(B, ctx) → v, ctx' where:
      v = evaluate(U, σ)
      ctx'.history = ctx.history ∪ {(B, v, t)}
      ctx'.topology = ctx.topology ∖ {B}
      constraints propagated outward from ∂U

## Refuse Rule

    B is active
    ────────────────────────────────────────
    refuse(B, ctx) → null, ctx' where:
      B.state = REFUSED
      ctx'.history = ctx.history ∪ {REFUSE_EVENT(B, t)}

## Collapse Rule

    op is a collapse operator, B is active
    ────────────────────────────────────────
    collapse(B, op, ctx) → B/~ where:
      B.state = COLLAPSED
      ~ is the equivalence relation of op

## Bind Rule

    cs is a ConstraintSet
    ────────────────────────────────────────
    bind(B, cs, ctx) → B', ctx where:
      B'.constraints = merge(B.constraints, cs)
      B'.state = BOUND
