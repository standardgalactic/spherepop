# Changelog

## [Unreleased]

### Added
- Initial C implementation: bubble lifecycle, history, constraints,
  provenance, scope, evaluator (event-driven), lexer, parser.
- Four primary operators: pop, refuse, collapse, bind.
- Event bus with standard listeners: provenance, history, admissibility,
  topology.
- Region VM opcode set (RegionInstruction — not stack instructions).
- SemanticObserver interface (returns AdmissibilityPoint, not scalar).
- Standard library stubs: math, geometry, topology, thermodynamics.
- Example programs: arithmetic, bubble nesting, provenance, recursive
  collapse, observerhood, action minimization, sheaf gluing.
- Full CMake build system with sanitizer support.
- Integration test suite.
- Grammar (EBNF), operational semantics specification.

### Design decisions recorded
- ConstraintRef as table index, not bitmask (relational constraints compose).
- AdmissibilityPoint returned by observers, not double (manifold, not score).
- Event dispatch before recursive evaluation (prevents evaluator ownership).
- graph allocation before event bus (bus interface shaped by bubble type).
