# Roadmap

## Phase 1 — Operational Core (current)
- [x] Bubble lifecycle: alloc, pop, refuse, collapse, bind
- [x] Event-driven evaluator
- [x] History recording
- [x] Constraint set (relational, not bitmask)
- [x] Provenance store
- [x] Scope as physical boundary
- [x] Recursive descent parser
- [x] Lexer

## Phase 2 — Runtime Depth
- [ ] Region allocator (memory.c — group allocations by bubble lifetime)
- [ ] GC cycle detection for closures
- [ ] Constraint propagation: full directed graph traversal
- [ ] Lazy event listeners (demand-driven provenance)
- [ ] Admissibility: Hamiltonian and full trajectory scoring
- [ ] Action functional: stationary history detection
- [ ] Bytecode compiler: AST → RegionInstruction

## Phase 3 — Geometry and Semantics
- [ ] Manifold representation for admissibility regions
- [ ] Curvature and boundary geometry
- [ ] Sheaf semantics: local-to-global consistency checking
- [ ] Category semantics: collapse functors
- [ ] Observerhood: full CLIO projection operator

## Phase 4 — Tooling
- [ ] Provenance viewer (CLI)
- [ ] Admissibility checker (depends on runtime stratum, not tools/)
- [ ] Topology exporter (graphviz)
- [ ] Literate extractor: monograph theorem → runtime test
- [ ] Static site generator

## Phase 5 — Visualizer
- [ ] Bubble renderer (WebGL/TypeScript)
- [ ] History timeline
- [ ] Admissibility panel
- [ ] Real-time pop event stream
