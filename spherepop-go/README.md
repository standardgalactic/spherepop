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
implementations. The fixture format is not the wire format. The canonical wire
encoding of a replayable world is SPHIST/1, specified byte for byte in
[`../experiments/flat/HISTORY-WIRE-V1.md`](../experiments/flat/HISTORY-WIRE-V1.md)
and implemented here by `EncodeHistory` and `DecodeHistory`. The decoder
rejects invalid UTF-8, truncation, impossible declared counts, unknown event
tags, invalid flags, non-canonical ordering of options and rules, and trailing
bytes. A fuzz test checks that it never panics and that every envelope it
accepts re-encodes to the same bytes:

```sh
go test -run '^$' -fuzz '^FuzzWireDecode$' -fuzztime 30s
```

`FNV1a64` is a stable conformance identifier and not a cryptographic digest.

Events carry their optional fields as pointers, so every path that stores or
returns an event copies those fields: proposal construction, history append,
`Events`, `EventsOfKind`, `Meld`, overlay creation, and state projection. A
caller that keeps a pointer it passed in, or modifies one it received, cannot
change the authoritative history. `immutability_guard_test.go` holds each of
these paths to that rule.

No derived operation introduces another primitive event kind. `Link` is a
`Bind`; `Unlink` is a `Refuse` of a prior binding; `Choice` is `Pop` plus
`Refuse`; `Merge` is `Bind` plus `Collapse`; and `SetMeta` is a distinguished
metadata `Bind` ignored by ordinary quotient rules.
