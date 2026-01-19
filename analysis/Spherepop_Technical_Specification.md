# Spherepop Technical Specification: Causal Sovereignty and the Portable History Protocol

---

## 1. Strategic Context: The Constitutional Rearchitecture of Digital Reality

The contemporary digital landscape is undergoing a critical transition from platform-centric architectures to history-first substrates. This document represents a fundamental re-engineering of the ontological substrate of digital interaction. Historically, institutional power has been maintained through a monopoly over temporal sequence—the exclusive authority to dictate the order, persistence, and interpretation of events. This structural monopoly enables platform decay and enshittification, where an **Incentive Inversion** occurs: once market saturation is reached, institutions maximize revenue yield by subordinating user utility to extraction efficiency. This document serves as a constitutional remedy to restore digital sovereignty by transforming causality from a proprietary asset into a public object.

By establishing the **Forkability of Time**, we redefine digital reality as a causal regime rather than a computational engine. In the current Hostile Interface paradigm, users are subjected to a monetization surface where every interaction is a behavioral probe. Spherepop proposes a **Legible World** where shifting from opaque, institutionally held state to verifiable, portable history eliminates the mute compulsion of institutional lock-in. When temporal sequence is no longer an institutional secret but a mechanically fixed invariant, digital sovereignty is returned to the participant.

---

## 2. Core Definitions and Domain Entities

The establishment of a universal standard for causal emigration requires stable, implementation-independent definitions.

| Entity | Constitutional Role |
|-------|---------------------|
| **Log (LID)** | An append-only sequence of events {e₀, e₁, …, eₙ} representing a unique causal trajectory. |
| **Proposal** | A client-authored intent or unordered request submitted for sequencing. |
| **Event** | A sequenced proposal combined with Arbiter-attested ordering metadata and cryptographic timestamps. |
| **Arbiter** | A partial function A : 𝓗 × 𝓟 → 𝓗 that selects a path through the causal manifold but does not create the space of possibilities. |
| **Portable History** | A serialized, canonically encoded representation of a log sufficient to deterministically replay semantic state. |
| **Checkpoint** | An optional state snapshot derived from replay; possesses no independent authority and serves only as an optimization. |

**Sovereignty vs. Optimization**

The distinction between Event (authority) and Checkpoint (optimization) preserves the Replay Invariant. Institutions typically control reality by owning state. In Spherepop, state is always reconstructible from history, preventing ownership of the present or past.

---

## 3. The Arbiter–Client Contract: Negative Rights and Non-Sovereignty

Spherepop constrains institutional power through **Negative Rights**—properties the Arbiter is forbidden to violate.

### Axiom C.1: Causal Monotonicity

For a given LID, the Arbiter shall never produce conflicting authoritative sequences. For any two histories H₁, H₂:

```
H₁ ⊑ H₂ or H₂ ⊑ H₁
```


Any fork at the same index constitutes a constitutional breach.

### Axiom C.2: Informational Non-Interference

The Arbiter may sequence a proposal but must preserve the semantic payload byte-for-byte under canonical encoding.

### Axiom C.3: Deterministic Latency (Heartbeat)

Sequencing acknowledgement must occur within a bounded public window. Refusal requires an explicit failure certificate. Indefinite silence is prohibited.

These axioms dismantle platform sovereignty by preventing reinterpretation, suppression, and silent censorship.

---

## 4. The Replay Invariant: State as a Deterministic Derivative

Spherepop inverts ontology from state-as-primitive to history-as-primitive.

For any deterministic evaluator 𝔈:

```
State_n = 𝔈(e₀, e₁, …, eₙ)
```


### Requirements for Correct Evaluators

1. **Pure Functional Behavior** — identical histories produce identical states.
2. **Implementation Independence** — all correct evaluators agree.
3. **History Primacy** — no hidden server-side indices permitted.

When history is portable, exit no longer implies social death.

---

## 5. The Exit Protocol: Normative Requirements for Causal Emigration

The Exit Protocol transfers temporal sovereignty and neutralizes institutional lock-in.

### Phase I: Total Log Export (E.1–E.4)

- **Completeness (E.1)** — includes e₀ through eₙ.
- **Order Preservation (E.2)** — canonical sequence maintained.
- **Verifiable Attestation (E.3)** — signed manifest with hash chain/Merkle root.
- **Non-Withholding (E.4)** — export is an inviolable right.

### Phase II: Verification and Continuity (V.1–V.3)

- **Integrity (V.1)** — Arbiter signature verified.
- **Causal Continuity (V.2)** — contiguous indices and hash validity.
- **Replay Determinism (V.3)** — reference evaluator yields identical terminal state.

### Phase III: Resumption of Reality (R.1–R.3)

- **Tip Agreement (R.1)** — new Arbiter binds to (LID, n, TipHash).
- **Continuation (R.2)** — new events begin at eₙ₊₁.
- **Fork Declaration (R.3)** — silent forks forbidden.

The protocol liberates history rather than merely regulating markets.

---

## 6. The Portable History Format (PHF): Standards for Fungible Realities

The PHF ensures worlds remain mechanically portable.

### Structural Conditions

- **[F.1] Canonical Encoding** — one-to-one mapping from history to bytes.
- **[F.2] Stable Identifiers** — IDs remain valid across institutions.
- **[F.3] Semantic Completeness** — no proprietary metadata required.
- **[F.4] Incremental Verification** — streaming validation via Merkle proofs.

Canonical encoding turns trust into arithmetic and eliminates interpretive discretion.

---

## 7. Theoretical Framework: The Geometry of Forkable Time

Spherepop replaces linear platform time with a public branching manifold.

### Formal Structure

- **Prefix Order:** Lawful migration preserves prefix relation H ⊑ H_new.
- **World Identity:** Worlds are equivalence classes [H]; equality is syntactic.
- **Migration as Isomorphism:** Exit induces id_H : H → H.
- **Arbiter as Path Selector:**

```
A : 𝓗 × 𝓟 → 𝓗
```


Time becomes forkable precisely because it becomes precise.

---

## 8. Conclusion: Implementation and Compliance

Spherepop ensures that digital worlds are carried by their participants rather than rented from institutions. By grounding sovereignty in mathematical invariants of history, the post-platform order becomes possible.

Digital reality is no longer a captive state. It is a portable, sovereign trajectory.

