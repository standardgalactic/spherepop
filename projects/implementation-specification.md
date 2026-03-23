Implementation Specification: The Spherepop Stratified Execution Environment

1. Architectural Foundations: History as Identity

The Spherepop execution environment represents a fundamental shift from state-based metaphysics to a "History as Identity" model. Traditional computation models a function (A \rightarrow B) as an ephemeral transition where the intermediate path is discarded. In contrast, Spherepop treats computation as a monotonically growing causal chain of irreversible commitments. Here, a process is not a point-in-time configuration, but the specific, ordered sequence of exclusions that produced it. The system is formalized as a Path Category \mathbb{P}(\Omega) where morphisms are finite sequences of commitments. Unlike traced monoidal categories, \mathbb{P}(\Omega) lacks inverses; identity is defined as the literal equality of ordered sequences, not as a homotopy class of paths.

The system state is defined by the pair (\Omega_t, H_t). This structure explicitly separates potentiality from actuality.

Component	Mathematical Structure	Temporal Role	Architectural Description
Residual Option Space (\Omega_t)	Unordered Set	Future Possibilities	A counterfactual record of available choices. It captures not just what is possible, but what could have happened but did not, allowing the system to distinguish between free selection and duress.
Commitment History (H_t)	Ordered List (Free Monoid \Omega^*)	Past Causal Chain	The authoritative, append-only record of actualized selections. It forms the monotonic "spine" of the process identity.

The Axiom of Irreversibility

The kernel is governed by the Axiom of Irreversibility: The world is constructed, not updated. Structural "undo" or "rollback" mechanisms are categorically impossible because \mathbb{P}(\Omega) contains no inverses. Auditability is not a feature but a topological necessity; because every decision—including the correction of a prior event—must be appended to H_t, the system maintains total provenance. This ensures that the system never "rewinds," instead moving forward through dense chains of irreversible commitments.

2. The Stratified Semantic Model: Calculus, Kernel, and View

The Spherepop architecture enforces a Stratified Semantics, a separation necessitated by the need to decouple mechanical causation from speculative observation. This stratification prevents semantic corruption by isolating the authoritative causal logic from non-authoritative projections.

Functional Requirements of the Layers

1. The Calculus Layer (Meaning): A preorder-enriched category where morphisms are monotone with respect to commitment. It defines the abstract logic of event-histories using \Pi and \Sigma types within an intensional dependent type theory core.
2. The Kernel Layer (Causation): The authoritative, deterministic interpreter of the append-only log. It sequences accepted events into the canonical history H_t without speculation.
3. The View Layer (Observation): A functorial projection from the Kernel. It supports snapshots, filtering, and speculative branching.

The Non-Interference Invariant

A foundational invariant prohibits feedback from the View layer into the Kernel. Because Views are allowed to be speculative, filtered, or reordered, any leak of View-data into the Kernel would induce semantic corruption. If the authoritative Kernel were to ingest a speculative View, the "History as Identity" would collapse into a hallucination. The Kernel must remain blind to the View to preserve causal discipline and determinism.

3. The Authoritative Kernel: Append-Only Logic and Log-Bound Processes

The Kernel is the sole "Source of Truth," managing the totally ordered, canonical record of events. It enforces the monotonic growth of H_t and ensures that once a Pop is sequenced, the associated option is permanently foreclosed in \Omega_t.

Log-Bound Process Identity

Process identity is determined by log prefixes rather than nominal identifiers.

1. Prefix Equivalence: Two processes are identical if and only if their histories H_t share an equivalent ordered sequence.
2. Forking as Branching: Branching a process is defined as branching an option-space prior to commitment (Source 11.2). The new process inherits the parent’s H_t as its prefix but begins a distinct trajectory in \Omega_t.
3. Historical Persistence: Identity is the log; so long as the append-only substrate exists, the process is inherently persistent and reconstructible.

Historical Compaction via the Collapse Operator

To manage log growth, the Kernel implements Historical Compaction via the Collapse operator.

* Collapse vs. Erasure: Unlike erasure, which deletes information, Collapse is a mathematical quotient (Source 5.3). It identifies distinctions that are no longer relevant for downstream action.
* Present Simplification: It simplifies the present active state by identifying equivalence classes of past events without falsifying or deleting the authoritative history. It "integrates out" fine-grained scaffolding while preserving the semantic outcome.

4. The Abstract Calculus: Implementing the Morphology of Choice

The Calculus layer defines the generating morphisms that express agency through the "Morphology of Choice."

* Pop: The fundamental act of selection. It removes x from \Omega_t and appends it to H_t.
* Bind: Imposes contextual restrictions or ordering constraints (e.g., a \prec b). It narrows \Omega_t without recording a commitment in H_t.
* Refuse: An ethical exclusion. Geometrically, Refuse_t \equiv Pop_t in the option space O. The distinction is only visible under the Accounting Functor (A), which records the ethical annotation without altering the mechanical interpretation of the Kernel.
* Merge: An equivalence induction using a Union-Find structure to identify irrelevance between two previously distinct paths.

The Mechanics of Commitment

The cost of world-construction is quantified through a discrete Lagrangian: L_t = \Delta\Omega_t + \lambda\Delta C_t where \Delta\Omega_t is the geometric reduction in optionality and \Delta C_t is the auxiliary cost. From this, we derive the Conjugate Momentum (\pi_t): \pi_t = -\frac{\partial L_t}{\partial \Omega} \pi_t measures Commitment: the rate at which future freedom is converted into settled reality. The Hamiltonian (H_t) represents the agent's "remaining maneuverability": H_t = \pi_t \Omega_t - L_t Agency is the disciplined reduction of H_t to convert unconstrained freedom into a defined world.

5. The View Layer: Non-Authoritative Projections and Observational Geometry

The View Layer provides "access without power," allowing observation of the history's shadow without contaminating the causal chain.

Replay and Observational State

The observable state S_{obs} is a non-authoritative projection derived by replaying the ledger from the root state s_0 using a composed sequence of transformations: S_{obs} = T_{xn} \circ \dots \circ T_{x_1}(s_0) This projection is a "shadow" of the path; the state itself is never the primary object, only the history that generates it.

Visualizing Worldhood: Nested Scopes

The UI paradigm must utilize Nested Scopes (Nested Bubbles/Spheres) to represent the topology of the world.

* Topology over Geometry: The UI respects inclusion and overlap rather than Euclidean distance.
* Depth as Commitment: Deeper nesting correlates to deeper commitment levels within a history.
* Sealed Regions: Refused regions are rendered as "sealed" volumes—visible as foreclosed possibilities but topologically inaccessible.

6. Functional Boundaries and Inter-Layer Communication

Communication between strata follows strict functorial relationships. The Kernel acts as the concrete realization of the Calculus's abstract morphisms.

The Proposal-Rejection Flow

1. Proposal: An agent proposes an event (e.g., a Pop).
2. Arbiter: The Arbiter (Source 15.3) exercises authority prior to sequencing, validating the proposal against the current \Omega_t.
3. Validation: The Calculus layer ensures the proposal does not violate existing Bind constraints.
4. Sequencing: Upon acceptance, the Kernel sequences the event into the append-only log. Once committed, the event is purely mechanical.

The Accounting Functor (A)

The Accounting Functor (A: O \to C) maps the geometric space of the Kernel to a category of costs and ethical weights. This allows the system to distinguish between a Pop and a Refuse for auditing purposes, preserving ethical integrity without contaminating the mechanical determinism of the Kernel transitions.

7. Implementation Roadmap and Operational Semantics

Implementation requires formal rigor to ensure type preservation across the stochastic and deterministic fragments of the environment.

Key Algorithms

* Normalization by Evaluation (NbE): Used to handle definitional equality within the intensional dependent type theory core.
* Bidirectional Type Checking: Ensures type preservation across all transitions, verifying that each Pop results in a valid sub-region of the option space.
* Union-Find Structure: Specifically required for managing equivalence classes during Merge and Collapse operations to maintain canonical representatives.

Metatheoretic Requirements

1. Confluence: Required for the deterministic fragment to ensure that all valid reduction paths result in the same historical state.
2. Stochastic Fragment: Transitions are modeled as a Markov kernel over terms. Type preservation must hold for the stochastic fragment, ensuring that even probabilistic "Choices" result in well-typed histories.

The "Joy of Spherepop" is found in this discipline: the realization of agency not through the maintenance of infinite freedom, but through the deliberate, irreversible construction of a finite path.

