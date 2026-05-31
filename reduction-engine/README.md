# Spherepop Reduction Engine

A reduction engine whose fundamental unit is a **Bubble**.

Every computation step produces exactly one of three named outcomes:

| Outcome | Meaning |
|---------|---------|
| **POP** | The bubble successfully reduced one instruction and continues (or exhausted its program and disappears). Normal forward progress. |
| **REFUSE** | An admissibility constraint was violated. The bubble closes without consuming the instruction. No crash — the expression remains available for another context. |
| **COLLAPSE** | The bubble commits a value upward into its parent's inbox and closes. The value escapes the local universe. |

This is not an actor model with fancy terminology. Pop/Refuse/Collapse are
first-class outcomes. Every reduction step is a named event. The
operating-system behaviors (scheduling, message passing, blocking) are
emergent from the reduction semantics rather than the foundation they sit on.

---

## Architecture

### Bubble states

```
OPEN      — ready or blocked, still reducible
POPPED    — program exhausted or halt executed
REFUSED   — admissibility gate rejected a reduction
COLLAPSED — bubble committed a value upward and closed
```

### The admissibility gate

Every instruction passes through `admissibility_gate()` before executing.
It returns:

- `POP` — admissible, proceed to execution
- `REFUSE` — constraint violated, close bubble without executing
- `None` — blocked (inbox empty but a sender may still arrive)

This is the **single architectural choke point** for constraint checking.
Adding new constraints means adding cases to the gate — not scattering
`if/else` logic through individual opcodes. CLIO-style projections,
RSVP-inspired accessibility fields, or type-theoretic constraints all
attach here.

### Bubble hierarchy

Bubbles nest. A bubble can spawn child bubbles. The kernel is the root.

```
kernel(
  (1:orchestrator:OPEN:pc=2:inbox=0)
    (2:worker:COLLAPSED:pc=3:inbox=0)  ↑42
)
```

A child that `collapse`s a value delivers it into the parent's inbox.
The parent can `recv` it. This is the return-value mechanism — a value
escaping from a local universe into the outer one.

### Scheduler

The scheduler is a simple round-robin by bubble id. Each tick:

1. Find the first OPEN bubble.
2. Run it through the admissibility gate.
3. If `REFUSE` → close the bubble, log the event.
4. If `None` (blocked) → try the next bubble.
5. If `POP` → execute the instruction, log the event.
6. If execution produces `COLLAPSE` → deliver value to parent, log the event.

---

## Files

| File | Purpose |
|------|---------|
| `spherepop.py` | Reference Python implementation — REPL, file runner, demo mode |
| `spherepop_engine.jsx` | Interactive React visualizer — step/run/pause, bubble cards, reduction log |

The Python file is the canonical implementation. The JSX visualizer is
pedagogically useful for watching the reduction sequence unfold in real time.

---

## Python usage

```bash
python spherepop.py            # interactive REPL
python spherepop.py --demo     # built-in demonstrations
python spherepop.py FILE.sph   # run a source file
```

### REPL commands

```
spawn NAME { instr; instr; ... }   create a top-level bubble
run [N]                            run until termination, or N ticks
tick                               one reduction step
ps                                 bubble table
tree                               bubble hierarchy
send BID VALUE                     inject a message from the kernel
mem BID                            show bubble memory
inbox BID                          show bubble inbox
log [N]                            show last N reductions (default 20)
help                               command reference
quit / exit                        leave
```

---

## Instruction set

```
print VALUE          emit value to stdout
set NAME VALUE       bind NAME to VALUE in local memory
add NAME VALUE       add VALUE to NAME (integer arithmetic)
mul NAME VALUE       multiply NAME by VALUE
div NAME VALUE       divide NAME by VALUE  — REFUSE if VALUE == 0
sqrt SRC [DST]       square root of SRC into DST — REFUSE if SRC < 0
send BID VALUE       deliver VALUE to bubble BID's inbox
recv NAME            receive one message into NAME; block if inbox empty
jnz NAME LABEL       jump to LABEL if NAME != 0
label NAME           define a jump target (consumed silently)
spawn NAME { ... }   create a child bubble inline
yield                voluntarily yield this tick (cooperative scheduling)
collapse VALUE       commit VALUE upward to parent's inbox; close bubble
halt                 close this bubble (POP)
```

---

## Example sessions

### Simple POP chain

```
spherepop> spawn counter { set x 0; add x 1; add x 1; add x 1; print x; halt }
spawned bubble id=1 name='counter'
spherepop> run
[1:counter] 3
spherepop> log
   clk  bubble          outcome   instruction                     detail
----------------------------------------------------------------------
     1  [1:counter   ]  POP       set x 0                         set x = 0
     2  [1:counter   ]  POP       add x 1                         add x: 0 + 1 = 1
     3  [1:counter   ]  POP       add x 1                         add x: 1 + 1 = 2
     4  [1:counter   ]  POP       add x 1                         add x: 2 + 1 = 3
     5  [1:counter   ]  POP       print x                         print → 3
     6  [1:counter   ]  POP       halt                            halt → POP
```

### REFUSE — sqrt of a negative number

```
spherepop> spawn bad { set x -4; sqrt x r; halt }
spherepop> run
spherepop> log
   1  [1:bad]  POP     set x -4   set x = -4
   2  [1:bad]  REFUSE  sqrt x r   admissibility gate: constraint violation → REFUSE
```

The bubble closes. No exception. No crash. The constraint was not satisfied
in this universe — the bubble simply could not reduce.

### COLLAPSE — value escapes upward

```
spherepop> spawn worker { set x 6; mul x 7; collapse x }
spherepop> run
spherepop> log
   1  [1:worker]  POP      set x 6    set x = 6
   2  [1:worker]  POP      mul x 7    mul x: 6 × 7 = 42
   3  [1:worker]  COLLAPSE collapse x  collapse 42
```

### Message passing — sender unblocks receiver

```
spherepop> spawn receiver { recv msg; print msg; collapse msg }
spawned bubble id=1 name='receiver'
spherepop> spawn sender { send 1 42; halt }
spawned bubble id=2 name='sender'
spherepop> run
[1:receiver] 42
spherepop> log
   1  [1:receiver]  REFUSE   recv msg    (inbox empty, no sender yet — blocked)
   2  [2:sender  ]  POP      send 1 42   send 42 → bubble 1
   3  [2:sender  ]  POP      halt        halt → POP
   4  [1:receiver]  POP      recv msg    recv msg ← 42
   5  [1:receiver]  POP      print msg   print → 42
   6  [1:receiver]  COLLAPSE collapse msg collapse 42
```

---

## Extension points

The architecture is designed for clean extension:

**New constraints** → add cases to `_admissibility_gate()`.
No other code changes required.

**Rewrite rules** → replace `program: list[Instruction]` with
`rules: list[Rule]` where each rule is a pattern → reduction.
The scheduler's job becomes "find first admissible match" rather than
"execute next instruction". Pop/Refuse/Collapse semantics are unchanged.

**Typed universes** → the gate receives the bubble's declared type
context and refuses reductions that violate it. A real-only bubble
refuses complex arithmetic. An integer bubble refuses floats.

**CLIO projections** → the gate consults a constraint set and computes
an admissible projection of the candidate reduction before allowing it.

**RSVP accessibility fields** → attach a scalar field to the bubble
hierarchy; the gate uses local field values to determine admissibility.
Bubbles in low-accessibility regions refuse more aggressively.

---

## Relationship to Spherepop

The Spherepop calculus treats constraint as the single primitive. A bubble
is an isolated computational cell whose reductions are governed by what is
admissible in its local universe. Pop, Refuse, and Collapse are the three
ways a reduction can resolve — not error conditions, not special cases, but
the complete vocabulary of outcomes.

This implementation is a minimal but faithful model of that calculus. It is
intended as a teaching kernel and as a foundation for the fuller Spherepop
system described in *Geometry, Cognition, and the Transparency of
Computation* (the Spherepop monograph).

---

*Author: Flyxion — Independent Researcher*
