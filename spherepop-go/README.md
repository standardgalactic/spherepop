# Spherepop Go

This module is an independent Go 1.22 implementation of the canonical
four-primitive Spherepop model in [`../SPEC.md`](../SPEC.md). It is not a port
of the Rust source and shares no implementation code with it.

The public package implements the closed event alphabet (`Pop`, `Refuse`,
`Bind`, `Collapse`), append-only histories, deterministic replay, atomic
proposal admission through an Arbiter, certified Collapse rules, derived
surface operations, structural Meld, and non-authoritative overlays.

Run its unit tests from this directory:

```sh
go test ./...
```

Run the shared conformance fixtures:

```sh
go run ./cmd/fixtures ../experiments/flat/fixtures
```

The fixture runner consumes the same JSON files as the Rust, Python, and C
implementations. It does not treat the fixture format as a canonical wire
format for arbitrary histories; `SPEC.md` deliberately leaves that stronger
serialization format unspecified.

No derived operation introduces another primitive event kind. `Link` is a
`Bind`; `Unlink` is a `Refuse` of a prior binding; `Choice` is `Pop` plus
`Refuse`; `Merge` is `Bind` plus `Collapse`; and `SetMeta` is a distinguished
metadata `Bind` ignored by ordinary quotient rules.
