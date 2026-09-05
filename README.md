# Spherepop

[Formal Logic](textbook/README.md)

[Recursive Containment and Deferred Closure](https://standardgalactic.github.io/spherepop/dynamics/spherepop_scope_dynamics.pdf)

* [Cognitive Topology](https://standardgalactic.github.io/spherepop/dynamics/Cognitive_Topology.pdf) — *Notes*

* [The Geometry of Nested Mental Bubbles](https://standardgalactic.github.io/spherepop/dynamics/) — *Audio Overview*

[Geometry, Cognition, and the Transparency of Computation](https://standardgalactic.github.io/spherepop/admissibility/spherepop-monograph.pdf)

* [The Geometry of Computation](https://standardgalactic.github.io/spherepop/admissibility/The_Geometry_of_Computation.pdf) — *Notes*

* [How Nested Bubbles Fix Flat Math](https://standardgalactic.github.io/spherepop/admissibility/) — *Audio Overview*

---
[Music](https://standardgalactic.github.io/spherepop/music/)

[Implementation in C](compiler/README.md)

---

[History as Identity](https://standardgalactic.github.io/spherepop/History%20as%20Identity.pdf)

[Structured Irreversibility](https://standardgalactic.github.io/spherepop/Structured%20Irreversibility.pdf)

**Spherepop is an event-driven, visual and formal framework for building meaning, computation, and structure through irreversible actions.**

Instead of starting from *sets, states, or representations*, Spherepop starts from **events**: things that happen once, change what is possible next, and leave an auditable history behind.

At its core, Spherepop replaces static foundations with a **constructive, history-based mereology** (part–whole relations built over time).

![](scope-intro.gif)

Play the [Spellpop mini-game](https://standardgalactic.github.io/spherepop/spellpop.html)

[Spherepop Game Engine](https://standardgalactic.github.io/spherepop/game-engine.html) — *Playable Demo*

[Same State, Different History](https://standardgalactic.github.io/spherepop/history-comparator.html) — *Interactive Comparator*

---

## What Spherepop Is

Spherepop is simultaneously:

- a **formal calculus** for event-sourced structure,
- a **visual programming model** based on nested scopes ("bubbles"),
- and a **foundational alternative** to set-theoretic thinking.

Meaning, identity, and computation arise from *what has been done*, not from timeless axioms.

![](stack-explode.gif)

---

## Core Ideas (Short Version)

### 1. Events before objects
Nothing exists by assumption. Objects, relations, and identities exist **only if introduced by events**.

### 2. Part–whole instead of membership
Spherepop replaces set membership (`∈`) with a **time-indexed part-of relation**, built incrementally from events.

### 3. Four primitive events, nothing else
All structure arises from a replayable log of irreversible operations built from exactly four primitives:

- **Pop** — commit to a specific option, removing it from what remains possible
- **Refuse** — document that an option is inadmissible, *without* removing it from what remains possible
- **Bind** — couple two elements as dependent, without identifying them
- **Collapse** — observe the history under a chosen rule, projecting it onto that rule's quotient space

Everything else — Sphere, Merge, Choice, Link, Unlink, Nest, SetMeta — is derived: each is a fixed composition of the four above, not an independent primitive. Merge, for instance, is nothing more than a Collapse of a Bind under a rule that identifies its two ends.

Two independently-generated histories can also be combined structurally via **Meld**, the free monoidal composition of histories — but this is a fact about how the history monoid composes, not a fifth primitive. Ordinary concurrency is already expressible through Bind alone.

Existence is historical, not axiomatic.

### 4. Identity is historical
Two things are the same if they have the same event history.
There is no notion of identity independent of construction.

### 5. No classical paradoxes
Russell-style paradoxes cannot arise because Spherepop has:

- no global membership relation,
- no unrestricted comprehension,
- no predicate-generated objects.

If something exists, you can point to the event that made it.

### 6. Scales with reality, not hypotheticals
Spherepop replaces power sets and hypothetical infinity with **linear event logs**.
Complexity grows with *what actually happens*, not with what could have happened.

---

## Spherepop as a Visual Language

Spherepop also exists as an **interactive visual system**.

- Expressions are drawn as **nested bubbles** (scopes).
- Each bubble represents a local context or subexpression.
- **Popping a bubble** explicitly evaluates that scope.
- Computation proceeds by deliberate traversal, not automation.

This makes scope, order of evaluation, and dependency *visible*.

---

## Why This Matters

Spherepop is motivated by a simple observation:

> Real systems—cognitive, computational, social—are not defined by states.
> They are defined by **irreversible history**.

By making commitment, exclusion, coupling, and observation explicit, Spherepop provides:

- a clearer model of computation,
- a more faithful account of agency,
- and a foundation aligned with how real systems evolve.

---

## Learn More

- [The History of Spherepop](https://standardgalactic.github.io/spherepop/The%20History%20of%20Spherepop.pdf)

- [Operational Mereology via Event-Sourced Semantics](https://standardgalactic.github.io/spherepop/Operational%20Mereology.pdf)

- [The Joy of Spherepop](https://standardgalactic.github.io/spherepop/Joy-of-Spherepop.pdf)

- [The Architecture of Meaning](architecture-of-meaning/README.md)

- [Platform Dynamics](analysis/README.md)

- **Demos and visuals:**
  See the animations and screenshots in this repository for interactive examples of nested scopes, popping, and collapse.

---

# ░▒▓ SPHEREPOP AT A GLANCE ▓▒░

```
Traditional                 Spherepop
─────────────────────────────────────────────────────
State                  →    History
Context Γ              →    Ordered Event History H
Assignment             →    Constructive Event
Mutation               →    Monotone Growth
Delete                 →    Refuse
Equality               →    Collapse
Substitution           →    Historical Substitution
Function               →    History Transformer
Type                   →    Refusal Structure
Proof                  →    Replayable History
Compilation            →    History Construction
Verification           →    Deterministic Replay
Concurrency            →    Bind-Coupled Proposal Streams
                             (Meld: structural composition
                              of independent histories)
```

## The Four Primitive Operations

```
            ○ Pop
             │
             ▼
        Choose History
             │
      ┌──────┴──────┐
      ▼             ▼
 Refuse ✕      Bind ○──────► Dependency
      │             │
      └──────┬──────┘
             ▼
       Collapse ◎────► Observed Quotient
                        (identification / Merge is the
                         special case where the rule
                         quotients bound elements together)
```

Meld — the structural, monoidal composition of two independent histories — sits outside this diagram: it combines logs rather than transforming one, and is not required to derive concurrency, which already follows from Bind.

## Historical Type Theory

```
Traditional

Γ ⊢ t : A

Static assumptions
        │
        ▼
Type checking


Spherepop

H ⊢ t : A

Historical replay
        │
        ▼
Admissible continuation
        │
        ▼
Extended history
```

## Types are Refusal Structures

```
Candidate Continuation

        │
        ▼

   ┌──────────────┐
   │ Refusal Type │
   └──────────────┘

      │     │     │
      │     │     │
      ▼     ▼     ▼

   Admit  Defer  Refuse
     │              │
     ▼              ▼

 Continue      Stop History
```

## Historical Identity

```
History A ─────────────┐
                       │
                       ▼
                  Collapse
                       ▲
                       │
History B ─────────────┘

Equality is discovered,
not assumed.
```

## The Kernel

```
Parse
  │
  ▼
Elaborate
  │
  ▼
Replay
  │
  ▼
Normalize
  │
  ▼
Verify
  │
  ▼
Commit
```

---

```
Programs  = Histories

Types     = Refusal Structures

Proofs    = Replayable Histories

Functions = History Transformers

Computation = Admissible Growth

Truth = Verified Construction
```

> **Spherepop:** *Computation is not the transformation of state, but the irreversible growth of an admissible history.*
