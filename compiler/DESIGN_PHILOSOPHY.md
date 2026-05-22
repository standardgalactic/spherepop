# Spherepop Design Philosophy

Spherepop is not a notation system, nor a formal calculus, nor a
philosophy of science in isolation. It is a theory of visible structure:
a systematic account of why constrained systems become intelligible
through the preservation of navigable relational structure across
transformations, and become unintelligible through its erasure.

## The Central Claim

A computation is not primarily a mapping from input to output.
It is a history: an ordered sequence of admissible transformation events
through which a structured possibility space is navigated from an initial
state to a terminal state. The terminal value is one feature of this
history, and an important one. But it is not the whole of the computation.

## Consequences for Implementation

1. Parentheses are erased in conventional evaluation. Bubbles are not.
   They are popped — transformed into recorded events.

2. The evaluator must not be a reduction machine that silently
   produces terminal values. It must be an event-forming system
   that records, at each step, what happened, under what constraints,
   and with what admissibility structure.

3. Provenance is not debugging metadata. It is the primary semantic
   content of a computation. Two computations with the same terminal
   value but different provenances are formally distinct objects under
   any collapse operator that preserves more than terminal values.

4. The choice of collapse operator is never arbitrary. It depends on
   what structural invariants are relevant to the use of the result.
   Erasing provenance forecloses making this assessment correctly.

## Against Opacity, Not Against Structure

The target of Spherepop's critique is abstraction that conceals
the operational geometry of what it abstracts — not abstraction
as such, not formalism as such, not compression as such.

The proposal is to compress in ways that preserve what matters for
understanding. The measure of what matters is the admissibility
structure of the computation.
