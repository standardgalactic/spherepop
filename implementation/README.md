# Spherepop (reference implementation)

This directory contains a small reference implementation of the Spherepop calculus.

The system is organized around an append-only event history. Operations do not directly modify state; instead they emit explicit events. The current state, at any moment, is obtained by replaying those events in order. There is no separate mutable store beyond the history itself.

To support exploration, the system allows speculative overlays. An overlay is a temporary sequence of events layered on top of an existing history. Overlays can be replayed to see their effects, compared against other histories, or discarded. Nothing becomes part of the authoritative history unless an overlay is explicitly committed.

Derived views such as snapshots, diffs, and hashes are produced by replaying a history or a history plus overlays. These views exist only for inspection and comparison. They do not affect the history and can always be recomputed.
