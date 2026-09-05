# SPHIST/1 canonical history envelope

SPHIST/1 is the portable, canonical encoding of the inputs needed to replay a Spherepop world. It encodes the initial option space, the certified Collapse-rule names, and the ordered primitive history. It does not encode a derived terminal state, a Collapse result, fixture expectations, or surface syntax.

Every integer is unsigned and big-endian. Every count and byte-string length is a 32-bit integer. Every object identifier is a 64-bit integer. Strings are length-prefixed UTF-8 without terminators or normalization. Decoders must reject invalid UTF-8, truncation, unknown event tags, invalid flags, trailing bytes, and non-canonical set ordering.

The byte sequence is:

```text
magic                 8 bytes: 53 50 48 49 53 54 31 00 ("SPHIST1\\0")
omega_count           u32
omega                 omega_count × u64, strictly increasing
rule_count            u32
rules                 rule_count × (u32 byte_length, UTF-8 bytes),
                      strictly increasing by UTF-8 byte sequence
event_count           u32
events                event_count × event record
```

An event record begins with a one-byte kind and then carries exactly the corresponding payload:

| Kind | Primitive | Payload |
|---:|---|---|
| `0` | Pop | `a: u64` |
| `1` | Refuse | `a: u64`, `has_b: u8` (`0` or `1`), optional `b: u64`, `reason: string` |
| `2` | Bind | `a: u64`, `b: u64`, `tag: string` |
| `3` | Collapse | `rule: string` |

Event position is canonical by array index and is therefore not repeated in the envelope. Derived forms must be desugared before encoding. Duplicate initial options and duplicate certified rules are invalid rather than being preserved as meaningless alternate encodings.

The conformance digest is 64-bit FNV-1a over the complete envelope, rendered as exactly 16 lowercase hexadecimal digits. It is a stable cross-implementation regression identifier, not a cryptographic authenticity or security mechanism. Applications requiring tamper evidence should authenticate the envelope separately.

Fixture `09_replay.json` is the normative example. Its 107-byte envelope has digest `bf4988c6e7a3c379`. A conforming runner must construct those bytes from the executed primitive history, compare the digest, decode the bytes without consulting the fixture event list, replay from a new state initialized only from the decoded option space, and obtain the original derived state.
