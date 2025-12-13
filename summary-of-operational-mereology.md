# Summary: Operational Mereology via Event-Sourced Semantics

---

## Core Thesis

The paper proposes **Spherepop** as an alternative foundation to set theory. It replaces timeless set membership with an **event-sourced mereology** (a theory of parts and wholes) grounded in **operational, replayable semantics**. Existence, identity, and structure are products of execution, not axioms.

---

## Key Ideas

### 1. From Sets to Parts
- Spherepop replaces set membership `∈` with a **time-indexed part-of relation** `≤ₜ`.
- Objects exist **only if introduced by events**: `POP`, `MERGE`, `LINK`, `COLLAPSE`.

### 2. Event-Sourced Construction
- All structure arises from an **append-only event log**.
- **Deterministic replay** reconstructs system state at any time.
- **Existence is historical**, not predicate- or axiom-based.

### 3. Temporal Identity
- Identity is defined by **handles plus event history**, not extensional content.
- Equality is **explicit and authoritative** (via `COLLAPSE`), never implicit.

### 4. Avoidance of Set-Theoretic Pathologies
- **Russell’s paradox** is inexpressible because Spherepop has:
  - No universal domain of objects.
  - No primitive membership relation.
  - No comprehension principle (predicates cannot create objects).
- **Power sets** are replaced by **on-demand queries**, avoiding exponential ontological commitments.

### 5. Computational Alignment
- Ontological size grows **linearly with the number of events**, not with hypothetical substructures.
- **Authority and disagreement** are first-class concepts, mediated procedurally by an arbiter.

### 6. Relation to Other Foundations
- **Category theory** becomes a **derived view** over replayed graphs, not an ontology.
- **Type theory** types are projections; **identity types** correspond to explicit `COLLAPSE` events.
- **ZF set theory axioms** are operationalized, realized as views, or eliminated where unnecessary.

### 7. Formalization
- Temporal mereological axioms are **induced by replay**, not postulated.
- A **small-step operational semantics** and **BNF grammar** are provided.
- A **sheaf-theoretic interpretation** clarifies compositionality and coherence in event-sourced structure.

---

## Conclusion

Spherepop relocates foundations from **abstract, timeless sets** to **executable, historical construction**. It preserves expressive power while eliminating paradoxes and aligning ontology with computational practice. Set theory, category theory, and type theory are retained as **descriptive tools**, not sources of existence.

---

**In short:** Spherepop replaces set theory with a **constructive, event-driven mereology**, making foundations operational, inspectable, and computationally realistic.