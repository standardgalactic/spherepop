# A Critique of Engagement-Optimized Systems and a Proposal for Merge-Aware Alternatives

## Abstract

This paper diagnoses a deep architectural incompatibility between the engagement-optimized feed systems of contemporary social platforms and the requirements for coherent personal memory. We argue that by treating content as irreducible, feed-ordered atomic instances, these systems structurally degrade meaning over time, a process we formalize as semantic drift. This architectural choice, driven by metric capture, creates a systemic inability to recognize equivalence, forcing a false dichotomy between duplication and erasure that fragments identity and prevents the accumulation of durable understanding.

Drawing on theoretical frameworks from cognitive science, including conceptual blending, and systems theory, such as autocatalytic closure, we demonstrate that these platforms are designed to operate above the semantic error threshold, a critical point beyond which information collapses into noise. As a formally grounded alternative, we propose a gallery-first, merge-aware architecture founded on the principles of event sourcing and irreversible operations. This model replaces mutable states with an authoritative history of events and introduces a merge operator as a primitive for semantic reconciliation. We contend that this alternative provides a formal guarantee for the conservation of meaning, not as an emergent or optimized property, but as a structural invariant of the system's design.

--------------------------------------------------------------------------------

## 1. Introduction: The Architectural Crisis of Digital Memory

The everyday experience of using social media is increasingly defined by a sense of dissonance. Users attempting to cultivate a personal timeline, maintain a visual archive, or simply revisit past moments find themselves in a constant battle against clutter, noise, and algorithmic intrusion. These frustrations are often dismissed as surface-level UI problems or the unavoidable cost of a free service. Such descriptions, while accurate, are incomplete. They are symptoms of a deep and fundamental architectural conflict.

The central claim of this paper is that this resistance is not accidental but reveals a fundamental incompatibility between engagement-optimized feed architectures and the requirements of coherent personal memory systems. At the heart of this incompatibility lies a collapse of distinctions where semantic objects, publication events, and interaction signals are fused into a single primitive: the post as a feed-ordered instance. Under these conditions, repetition becomes duplication, accumulation is impossible, and semantic drift is inevitable.

This paper is structured in four parts. Part I diagnoses the specific structural failures of engagement-optimized architectures, beginning with their foundational economic logic and tracing its consequences through the system's core data model to the fragmentation of user identity. Part II grounds this critique in theoretical frameworks from cognitive science and systems theory, explaining why the architectural choices of feed-based systems are so profoundly dissonant with human sense-making and the general conditions for informational life. Part III proposes a concrete, formally grounded alternative: a gallery-first, merge-aware architecture built on an event-sourced ontology that guarantees the conservation of meaning by design. Finally, Part IV addresses common objections to this proposal and concludes by synthesizing the paper's central argument: that meaning cannot be optimized into existence, but must be architecturally conserved.

--------------------------------------------------------------------------------

## Part I: The Structural Failures of Engagement-Optimized Architectures

## 2. The Logic of Metric Capture: Goodhart's Law in Social Platforms

To understand the architectural failures of modern social platforms, one must first grasp the economic and behavioral principle that drives them: Goodhart’s Law, which states, "When a measure becomes a target, it ceases to be a good measure." This principle provides a unifying explanation for a range of systemic degradations observed in engagement-driven systems. Metrics such as likes, comments, and follows were originally introduced as proxies for genuine user interest, relevance, and social connection. However, once these proxies were elevated from descriptive signals to optimization targets, their informational value collapsed.

This metric capture transforms the platform's social dynamics. In domains like dating or professional collaboration, which rely on the fidelity of signals like identity and expressions of interest, the effects are particularly corrosive. An architecture optimized for engagement systematically rewards content and accounts that elicit rapid, affective responses, regardless of authenticity. As users adapt, identity ceases to be a stable representation of a person and becomes a performance optimized for attention capture.

This dynamic gives rise to false addressability, a phenomenon where users are constantly signaled as being addressed without being the intended recipient of communication. Broadcast tags create the appearance of direct address while functioning as indiscriminate tools for maximizing reach. Users rationally adapt, learning to discount notifications and treat communication as presumptively performative. These behavioral pathologies are not bugs; they are the intended consequences of a deeply flawed technical architecture.

## 3. Architectural Collapse: Feed-Ordered Atoms and Semantic Dilution

The behavioral effects of metric capture are implemented and enforced by a foundational architectural decision: the treatment of posts as irreducible, feed-ordered atoms. Each post is an immutable instance whose identity is fixed by its position in a feed.

Because the system lacks any mechanism for recognizing equivalence across instances, convergence is impossible. Reposting creates duplication rather than reinforcement. This forces a false dichotomy between duplication and erasure, preventing cumulative meaning. Reactions and annotations that should accrete around a stable artifact instead disperse across ephemeral instances, producing semantic dilution. The more an object is resurfaced, the less coherent its meaning becomes.

This mirrors distributed systems without merge operations, which inevitably diverge. On social platforms, this divergence manifests as interpretive drift and identity fragmentation.

## 4. The Fragmentation of Identity: The Cover Photo Paradox

The Cover Photo Paradox illustrates this failure in everyday use. A single photograph used as both a cover image and a timeline post is treated as two unrelated entities. Each accumulates separate reactions and histories, forcing the user to manually reconcile what the system refuses to unify.

To preserve order, the user must delete history. To preserve history, the user must tolerate incoherence. This false choice reveals a systemic failure to distinguish appearance from essence. Identity surfaces are multiplied while the underlying object is fragmented.

## 5. The Semantic Error Threshold: Formalizing the Collapse of Meaning

The semantic error threshold describes the point beyond which variation overwhelms correction, causing informational collapse. Engagement-optimized systems are designed to operate above this threshold. They maximize variation through duplication while lacking any error-correction mechanism such as merge.

This guarantees semantic drift. Just as biological systems fail without replication fidelity, semantic systems fail when identity cannot be preserved under transformation. Engagement-optimized platforms are therefore formally incapable of serving as durable memory substrates.

--------------------------------------------------------------------------------

## Part II: Theoretical Foundations for Meaning and Memory

## 6. Cognitive Dissonance: Human Sense-Making vs. Feed Dynamics

Conceptual Blending Theory shows that meaning requires stable shared structure. Feed-based systems produce combinatorial proliferation without convergence, fragmenting meaning rather than deepening it.

Remix culture explains why feeds feel expressive but fail as archives. Remix thrives on ephemerality; memory requires persistence.

Finally, engagement optimization is hostile to analogy and essence recognition. By rewarding surface novelty, it disrupts the mind’s ability to recognize sameness across variation. Constant engagement prompts force a shift from recognition to response, undermining memory formation.

## 7. Semantic Organisms: Informational Closure as a Prerequisite for Durable Meaning

The Substrate Independent Thinking Hypothesis defines life by organizational closure. Durable semantic systems must preserve identity through regulated transformation.

Feed-based architectures lack closure. Transformation always leads to divergence. Without merge, identity cannot persist. These systems are informationally equivalent to organisms without error correction and are destined to collapse into noise.

--------------------------------------------------------------------------------

## Part III: A Gallery-First, Merge-Aware Alternative

## 8. An Ontological Shift: From Mutable States to Irreversible Events

A viable alternative requires an ontological shift from mutable state to irreversible event history. Meaning arises from committed actions, not overwritable snapshots.

In an event-sourced system, history is authoritative. State is derived. This guarantees accountability, auditability, and the authority of the past, enabling systems to distinguish objects from appearances.

## 9. Core Principles of Merge-Aware Design

### Persistent Objects over Atomic Instances

Semantic objects are primary. Feeds are projections. This restores a stable generic space for meaning.

### The Merge Operator as a Reconciliation Primitive

Merge enables convergence after divergence. It is the error-correction mechanism that allows identity to persist through transformation.

### Identity Surfaces as Projections

Profile pictures and cover photos become views onto shared objects. Engagement accumulates on essence, not instances.

### State Separation: Viewing, Interaction, and Annotation

Viewing, reacting, and annotating are separated. Recognition is restored. Performance is no longer mandatory.

## 10. Formal Guarantees: Preventing Semantic Drift by Construction

Semantic drift occurs when meaning variance grows unbounded across instances. Merge-aware systems prevent this by ensuring all interactions refer to a single semantic object.

Corollary: Any system that optimizes engagement via instance-level metrics while lacking merge over semantic equivalence classes cannot conserve meaning over time.

--------------------------------------------------------------------------------

## Part IV: Objections and Responses

## 11. Addressing Architectural and Behavioral Objections

Chronological streams are valid for real-time events but unsuitable as memory substrates.

Ephemerality should be optional, not enforced.

Scalability without coherence merely scales entropy.

Merge complexity is preferable to merge impossibility. Offering merge restores agency.

--------------------------------------------------------------------------------

## 12. Conclusion: Meaning as an Architectural Property

Engagement-optimized systems fail because they treat content as disposable instances. This guarantees semantic drift.

A gallery-first, merge-aware, event-sourced architecture conserves meaning by design. Meaning is not emergent or optimizable. It is a structural invariant.

Meaning cannot be optimized into existence. It must be conserved by design.

--------------------------------------------------------------------------------

## References

Ammann, N. (2024). How to Avoid Two AI Catastrophes: Domination and Chaos.

Appel, A. W. (1997). Foundational proof-carrying code.

Brand, S. (1994). How Buildings Learn.

Bush, V. (1945). As We May Think.

Campbell, D. T. (1979). Assessing the impact of planned social change.

Church, A. (1936). An unsolvable problem of elementary number theory.

Doctorow, C. (2023). The enshittification of TikTok.

Eigen, M. (1971). Self-organization of matter.

Fauconnier, G., and Turner, M. (2002). The Way We Think.

Floridi, L. (2013). The Ethics of Information.

Goodhart, C. A. E. (1975). Problems of monetary management.

Hofstadter, D. R., and Sander, E. (2013). Surfaces and Essences.

Hutchins, E. (1995). Cognition in the Wild.

Kauffman, S. A. (1993). The Origins of Order.

Maturana, H. R., and Varela, F. J. (1980). Autopoiesis and Cognition.

Meijer, E. (2011). The duality of computation.

Necula, G. C. (1997). Proof-carrying code.

Ostrom, E. (1990). Governing the Commons.

Russell, S. (2019). Human Compatible.

Simon, H. A. (1962). The Architecture of Complexity.

Tufekci, Z. (2015). Algorithmic harms beyond Facebook and Google.

Turing, A. M. (1936). On computable numbers.

Turner, M. (2014). The Origin of Ideas.

Wittgenstein, L. (1953). Philosophical Investigations.

Zuboff, S. (2019). The Age of Surveillance Capitalism.