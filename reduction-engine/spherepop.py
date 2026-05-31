#!/usr/bin/env python3
"""
Spherepop Reduction Engine
==========================

A reduction engine whose fundamental unit is a Bubble.

Every computation step produces exactly one of three outcomes:

    POP      — the bubble successfully reduced one instruction and continues
                (or has exhausted its program and disappears)
    REFUSE   — an admissibility constraint was violated; the bubble closes
                without consuming the offending instruction; no crash
    COLLAPSE — the bubble commits a value upward into its parent's inbox
                and closes; the value escapes the local universe

The admissibility gate sits between the scheduler and execution. Every
instruction passes through it. This is where constraint checking lives.
The gate is the single architectural choke point — not scattered if/else
logic inside each opcode.

Bubble states
-------------
    OPEN      — ready or blocked, still reducible
    POPPED    — program exhausted or halt executed
    REFUSED   — admissibility gate rejected a reduction
    COLLAPSED — bubble committed a value upward and closed

Instruction set
---------------
    print VALUE          emit value to stdout
    set NAME VALUE       bind NAME to VALUE in local memory
    add NAME VALUE       add VALUE to NAME (integer)
    mul NAME VALUE       multiply NAME by VALUE (integer)
    div NAME VALUE       divide NAME by VALUE — REFUSE if VALUE == 0
    sqrt SRC [DST]       square root of SRC into DST (or SRC) — REFUSE if SRC < 0
    send BID VALUE       deliver VALUE to bubble BID's inbox
    recv NAME            receive one message into NAME; block if inbox empty
    jnz NAME LABEL       jump to LABEL if NAME != 0
    label NAME           define a jump target (consumed silently)
    spawn NAME { ... }   create a child bubble with given program
    yield                voluntarily yield this tick (cooperative scheduling)
    collapse VALUE       commit VALUE upward to parent; close this bubble
    halt                 close this bubble (POP)

Usage
-----
    python spherepop.py            # interactive REPL
    python spherepop.py --demo     # run built-in demonstration
    python spherepop.py FILE.sph   # run a source file

REPL commands
-------------
    spawn NAME { ... }   create a top-level bubble
    run [N]              run until all bubbles terminate, or for N ticks
    tick                 single reduction step
    ps                   show bubble table
    tree                 show bubble hierarchy
    send BID VALUE       inject a message from the kernel
    mem BID              show bubble memory
    inbox BID            show bubble inbox
    log [N]              show last N reduction events (default 20)
    help                 show this help
    quit / exit          leave the REPL
"""

from __future__ import annotations

import math
import shlex
import sys
from collections import deque
from dataclasses import dataclass, field
from typing import Deque, Optional

# ---------------------------------------------------------------------------
# Outcome and state constants
# ---------------------------------------------------------------------------

class Outcome:
    POP      = "POP"
    REFUSE   = "REFUSE"
    COLLAPSE = "COLLAPSE"

class BubbleState:
    OPEN      = "OPEN"
    POPPED    = "POPPED"
    REFUSED   = "REFUSED"
    COLLAPSED = "COLLAPSED"

# ---------------------------------------------------------------------------
# Data structures
# ---------------------------------------------------------------------------

@dataclass
class Instruction:
    op:   str
    args: list[str]
    raw:  str

@dataclass
class ReductionEvent:
    clock:     int
    bubble_id: int
    name:      str
    outcome:   str
    instr:     str
    detail:    str
    value:     object = None

    def __str__(self) -> str:
        val_part = f"  value={self.value!r}" if self.value is not None else ""
        return (
            f"  {self.clock:>4}  [{self.bubble_id}:{self.name:<10}]  "
            f"{self.outcome:<8}  {self.instr:<30}  {self.detail}{val_part}"
        )

@dataclass
class Bubble:
    id:        int
    name:      str
    parent_id: Optional[int]
    children:  list[int]        = field(default_factory=list)
    inbox:     Deque            = field(default_factory=deque)
    memory:    dict             = field(default_factory=dict)
    program:   list[Instruction]= field(default_factory=list)
    labels:    dict[str, int]   = field(default_factory=dict)
    pc:        int              = 0
    state:     str              = BubbleState.OPEN
    collapsed_value: object     = None

    def __post_init__(self) -> None:
        for i, instr in enumerate(self.program):
            if instr.op == "label" and instr.args:
                self.labels[instr.args[0]] = i

    def bubble_view(self) -> str:
        return f"({self.id}:{self.name}:{self.state}:pc={self.pc}:inbox={len(self.inbox)})"

# ---------------------------------------------------------------------------
# Parsing
# ---------------------------------------------------------------------------

def parse_program(src: str) -> list[Instruction]:
    code: list[Instruction] = []
    for raw in src.split(";"):
        raw = raw.strip()
        if not raw:
            continue
        try:
            parts = shlex.split(raw)
        except ValueError:
            parts = raw.split()
        if not parts:
            continue
        code.append(Instruction(op=parts[0], args=parts[1:], raw=raw))
    return code

def parse_spawn_command(line: str) -> tuple[str, str]:
    """Parse: spawn NAME { instruction; ... }"""
    rest = line[len("spawn "):].strip()
    if "{" not in rest or not rest.endswith("}"):
        raise SyntaxError(
            "spawn syntax: spawn NAME { instruction; instruction; ... }"
        )
    name, body = rest.split("{", 1)
    name = name.strip()
    body = body[:-1].strip()
    if not name:
        raise SyntaxError("spawn requires a process name")
    return name, body

def coerce_value(token: str) -> int | float | str:
    try:
        return int(token)
    except ValueError:
        pass
    try:
        return float(token)
    except ValueError:
        pass
    if token.startswith('"') and token.endswith('"'):
        return token[1:-1]
    return token

# ---------------------------------------------------------------------------
# Engine
# ---------------------------------------------------------------------------

class Engine:
    """
    The outer bubble (kernel). Owns all bubbles, the scheduler queue,
    the clock, and the reduction log.
    """

    def __init__(self) -> None:
        self.bubbles:  dict[int, Bubble] = {}
        self.next_id:  int               = 1
        self.clock:    int               = 0
        self.log:      list[ReductionEvent] = []

    # --- Bubble lifecycle --------------------------------------------------

    def spawn(self, name: str, src: str, parent_id: Optional[int] = None) -> int:
        bid = self.next_id
        self.next_id += 1
        program = parse_program(src)
        bubble = Bubble(id=bid, name=name, parent_id=parent_id, program=program)
        self.bubbles[bid] = bubble
        if parent_id is not None and parent_id in self.bubbles:
            self.bubbles[parent_id].children.append(bid)
        return bid

    def get(self, bid: int) -> Bubble:
        try:
            return self.bubbles[bid]
        except KeyError:
            raise ValueError(f"no bubble with id {bid}") from None

    def send(self, bid: int, value: object) -> None:
        bubble = self.get(bid)
        bubble.inbox.append(value)

    # --- Scheduling --------------------------------------------------------

    def has_open(self) -> bool:
        return any(b.state == BubbleState.OPEN for b in self.bubbles.values())

    def tick(self) -> Optional[ReductionEvent]:
        """
        One reduction step: find the first OPEN bubble (by id), run its
        next admissible reduction. Returns the ReductionEvent or None.
        """
        self.clock += 1
        candidates = sorted(
            (b for b in self.bubbles.values() if b.state == BubbleState.OPEN),
            key=lambda b: b.id,
        )
        for bubble in candidates:
            event = self._reduce(bubble)
            if event is not None:
                self.log.append(event)
                return event
        return None

    def run(self, max_ticks: Optional[int] = None) -> None:
        count = 0
        while self.has_open():
            if max_ticks is not None and count >= max_ticks:
                break
            event = self.tick()
            if event is None:
                break
            count += 1

    # --- Core reduction ----------------------------------------------------

    def _reduce(self, bubble: Bubble) -> Optional[ReductionEvent]:
        """
        Attempt one reduction on bubble. Returns a ReductionEvent or None
        (None means this bubble is blocked; try the next one).
        """
        if bubble.pc >= len(bubble.program):
            bubble.state = BubbleState.POPPED
            return ReductionEvent(
                clock=self.clock, bubble_id=bubble.id, name=bubble.name,
                outcome=Outcome.POP, instr="(end)", detail="program exhausted → POP",
            )

        instr = bubble.program[bubble.pc]

        # Labels are consumed silently — not a reduction event
        if instr.op == "label":
            bubble.pc += 1
            return self._reduce(bubble)

        # Admissibility gate
        gate = self._admissibility_gate(bubble, instr)

        if gate == Outcome.REFUSE:
            bubble.state = BubbleState.REFUSED
            return ReductionEvent(
                clock=self.clock, bubble_id=bubble.id, name=bubble.name,
                outcome=Outcome.REFUSE, instr=instr.raw,
                detail="admissibility gate: constraint violation → REFUSE",
            )

        if gate is None:
            # Blocked: inbox empty, no sender can unblock us
            return None

        # Execute
        bubble.pc += 1
        return self._execute(bubble, instr)

    def _admissibility_gate(self, bubble: Bubble, instr: Instruction) -> Optional[str]:
        """
        Returns:
            Outcome.POP     — admissible, proceed
            Outcome.REFUSE  — constraint violated, close bubble
            None            — blocked (inbox empty but a sender may arrive)
        """
        op, args = instr.op, instr.args

        if op == "sqrt":
            val = self._resolve(bubble, args[0]) if args else 0
            if isinstance(val, (int, float)) and val < 0:
                return Outcome.REFUSE

        if op == "div":
            divisor = self._resolve(bubble, args[1]) if len(args) > 1 else 0
            try:
                if float(divisor) == 0.0:
                    return Outcome.REFUSE
            except (ValueError, TypeError):
                return Outcome.REFUSE

        if op == "recv":
            if not bubble.inbox:
                # Check whether any OPEN bubble has a pending send targeting us
                has_sender = any(
                    other.state == BubbleState.OPEN and any(
                        i.op == "send" and i.args and i.args[0] == str(bubble.id)
                        for i in other.program[other.pc:]
                    )
                    for other in self.bubbles.values()
                    if other.id != bubble.id
                )
                if has_sender:
                    return None          # blocked, not refused
                return Outcome.REFUSE    # no sender ever coming → REFUSE

        return Outcome.POP

    def _execute(self, bubble: Bubble, instr: Instruction) -> ReductionEvent:
        op, args = instr.op, instr.args
        rv = lambda t: self._resolve(bubble, t)

        def event(outcome, detail, value=None):
            return ReductionEvent(
                clock=self.clock, bubble_id=bubble.id, name=bubble.name,
                outcome=outcome, instr=instr.raw, detail=detail, value=value,
            )

        if op == "halt":
            bubble.state = BubbleState.POPPED
            return event(Outcome.POP, "halt → POP")

        if op == "yield":
            return event(Outcome.POP, "yield (cooperative)")

        if op == "collapse":
            val = rv(args[0]) if args else None
            bubble.state = BubbleState.COLLAPSED
            bubble.collapsed_value = val
            detail = f"collapse {val!r}"
            if bubble.parent_id is not None:
                parent = self.bubbles.get(bubble.parent_id)
                if parent and parent.state == BubbleState.OPEN:
                    parent.inbox.append(val)
                    detail += f" → inbox of bubble {bubble.parent_id}"
            return event(Outcome.COLLAPSE, detail, value=val)

        if op == "print":
            val = rv(args[0]) if args else ""
            print(f"[{bubble.id}:{bubble.name}] {val}")
            return event(Outcome.POP, f"print → {val!r}", value=val)

        if op == "set":
            if len(args) < 2:
                bubble.state = BubbleState.REFUSED
                return event(Outcome.REFUSE, "set: requires 2 arguments")
            bubble.memory[args[0]] = rv(args[1])
            return event(Outcome.POP, f"set {args[0]} = {bubble.memory[args[0]]!r}")

        if op == "add":
            if len(args) < 2:
                bubble.state = BubbleState.REFUSED
                return event(Outcome.REFUSE, "add: requires 2 arguments")
            old = _to_num(bubble.memory.get(args[0], 0))
            bubble.memory[args[0]] = old + _to_num(rv(args[1]))
            return event(Outcome.POP, f"add {args[0]}: {old} + {rv(args[1])} = {bubble.memory[args[0]]}")

        if op == "mul":
            if len(args) < 2:
                bubble.state = BubbleState.REFUSED
                return event(Outcome.REFUSE, "mul: requires 2 arguments")
            old = _to_num(bubble.memory.get(args[0], 1))
            bubble.memory[args[0]] = old * _to_num(rv(args[1]))
            return event(Outcome.POP, f"mul {args[0]}: {old} × {rv(args[1])} = {bubble.memory[args[0]]}")

        if op == "div":
            # Gate already checked for zero
            old = _to_num(bubble.memory.get(args[0], 0))
            den = _to_num(rv(args[1]))
            bubble.memory[args[0]] = old / den
            return event(Outcome.POP, f"div {args[0]}: {old} / {den} = {bubble.memory[args[0]]}")

        if op == "sqrt":
            src = _to_num(rv(args[0]))
            dst = args[1] if len(args) > 1 else args[0]
            result = math.sqrt(src)
            bubble.memory[dst] = result
            return event(Outcome.POP, f"sqrt({src}) = {result}")

        if op == "send":
            if len(args) < 2:
                bubble.state = BubbleState.REFUSED
                return event(Outcome.REFUSE, "send: requires target id and value")
            target_id = int(_to_num(rv(args[0])))
            val = rv(args[1])
            target = self.bubbles.get(target_id)
            if target is None:
                bubble.state = BubbleState.REFUSED
                return event(Outcome.REFUSE, f"send: no bubble {target_id} → REFUSE")
            target.inbox.append(val)
            return event(Outcome.POP, f"send {val!r} → bubble {target_id}")

        if op == "recv":
            # Gate ensures inbox is non-empty here
            val = bubble.inbox.popleft()
            bubble.memory[args[0]] = val
            return event(Outcome.POP, f"recv {args[0]} ← {val!r}", value=val)

        if op == "jnz":
            if len(args) < 2:
                bubble.state = BubbleState.REFUSED
                return event(Outcome.REFUSE, "jnz: requires NAME LABEL")
            val = _to_num(rv(args[0]))
            label = args[1]
            if val != 0:
                if label not in bubble.labels:
                    bubble.state = BubbleState.REFUSED
                    return event(Outcome.REFUSE, f"jnz: unknown label {label!r} → REFUSE")
                bubble.pc = bubble.labels[label]
                return event(Outcome.POP, f"jnz {args[0]}={val} → label {label!r}")
            return event(Outcome.POP, f"jnz {args[0]}={val} → no jump")

        if op == "spawn":
            # spawn NAME { ... }  inside a program
            name = args[0] if args else "child"
            body = " ".join(args[1:]).strip().lstrip("{").rstrip("}")
            child_id = self.spawn(name, body, parent_id=bubble.id)
            bubble.memory[name] = child_id
            return event(Outcome.POP, f"spawn child bubble {child_id} ({name})")

        # Unknown opcode
        bubble.state = BubbleState.REFUSED
        return event(Outcome.REFUSE, f"unknown op {op!r} → REFUSE")

    # --- Value resolution --------------------------------------------------

    def _resolve(self, bubble: Bubble, token: str) -> object:
        if token in bubble.memory:
            return bubble.memory[token]
        if token.startswith('"') and token.endswith('"'):
            return token[1:-1]
        return coerce_value(token)

    # --- Display -----------------------------------------------------------

    def process_table(self) -> str:
        header = f"{'id':<4}  {'name':<12}  {'state':<10}  {'pc':<6}  {'inbox':<6}  {'children'}"
        rows = [header, "-" * 60]
        for b in sorted(self.bubbles.values(), key=lambda b: b.id):
            children = ",".join(str(c) for c in b.children) or "—"
            row = (
                f"{b.id:<4}  {b.name:<12}  {b.state:<10}  "
                f"{b.pc:<6}  {len(b.inbox):<6}  {children}"
            )
            if b.collapsed_value is not None:
                row += f"  ↑{b.collapsed_value!r}"
            rows.append(row)
        return "\n".join(rows)

    def tree(self, bid: Optional[int] = None, indent: int = 0) -> str:
        if not self.bubbles:
            return "kernel()"
        if bid is None:
            # roots: bubbles with no parent
            roots = [b for b in self.bubbles.values() if b.parent_id is None]
            lines = ["kernel("]
            for root in sorted(roots, key=lambda b: b.id):
                lines.append("  " + self.tree(root.id, indent=2))
            lines.append(")")
            return "\n".join(lines)
        bubble = self.bubbles[bid]
        prefix = " " * indent
        node = f"{prefix}{bubble.bubble_view()}"
        if bubble.children:
            child_lines = [self.tree(c, indent + 2) for c in sorted(bubble.children)]
            return node + "\n" + "\n".join(child_lines)
        return node

    def show_log(self, n: int = 20) -> str:
        entries = self.log[-n:]
        if not entries:
            return "(no reductions yet)"
        header = f"  {'clk':>4}  {'bubble':<14}  {'outcome':<8}  {'instruction':<30}  detail"
        lines = [header, "-" * 90]
        lines.extend(str(e) for e in entries)
        return "\n".join(lines)

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _to_num(value: object) -> int | float:
    if isinstance(value, (int, float)):
        return value
    try:
        return int(str(value))
    except ValueError:
        try:
            return float(str(value))
        except ValueError:
            raise ValueError(f"expected number, got {value!r}") from None

# ---------------------------------------------------------------------------
# REPL
# ---------------------------------------------------------------------------

HELP = """
Spherepop Reduction Engine — REPL commands
-------------------------------------------
  spawn NAME { instr; instr; ... }   create a top-level bubble
  run [N]                            run until termination, or N ticks
  tick                               one reduction step
  ps                                 bubble table
  tree                               bubble hierarchy
  send BID VALUE                     inject a message from the kernel
  mem BID                            show bubble memory
  inbox BID                          show bubble inbox
  log [N]                            show last N reductions (default 20)
  help                               this help
  quit / exit                        leave

Instruction set
---------------
  print VALUE          emit value
  set NAME VALUE       bind variable
  add NAME VALUE       integer addition in place
  mul NAME VALUE       integer multiplication in place
  div NAME VALUE       division — REFUSE if divisor is zero
  sqrt SRC [DST]       square root — REFUSE if SRC < 0
  send BID VALUE       deliver to another bubble's inbox
  recv NAME            receive one message, or block / REFUSE
  jnz NAME LABEL       conditional jump
  label NAME           jump target
  spawn NAME { ... }   create a child bubble
  yield                cooperative yield
  collapse VALUE       commit value upward and close
  halt                 close this bubble (POP)

Reduction outcomes
------------------
  POP      — successful reduction; bubble continues or closes normally
  REFUSE   — admissibility gate rejected the instruction; bubble closes
             without crashing; the expression remains available
  COLLAPSE — bubble commits a value to its parent's inbox and closes
""".strip()

def run_repl_command(engine: Engine, line: str) -> None:
    line = line.strip()
    if not line:
        return

    if line in ("help", "?"):
        print(HELP)
        return

    if line == "ps":
        print(engine.process_table())
        return

    if line == "tree":
        print(engine.tree())
        return

    if line == "tick":
        event = engine.tick()
        if event:
            print(event)
        else:
            print("(no ready bubbles)")
        return

    if line.startswith("run"):
        parts = line.split()
        max_ticks = int(parts[1]) if len(parts) == 2 else None
        engine.run(max_ticks)
        return

    if line.startswith("spawn "):
        name, body = parse_spawn_command(line)
        bid = engine.spawn(name, body)
        print(f"spawned bubble id={bid} name={name!r}")
        return

    if line.startswith("send "):
        parts = shlex.split(line)
        if len(parts) != 3:
            raise SyntaxError("send syntax: send BID VALUE")
        engine.send(int(parts[1]), coerce_value(parts[2]))
        return

    if line.startswith("mem "):
        bid = int(line.split()[1])
        print(engine.get(bid).memory)
        return

    if line.startswith("inbox "):
        bid = int(line.split()[1])
        print(list(engine.get(bid).inbox))
        return

    if line.startswith("log"):
        parts = line.split()
        n = int(parts[1]) if len(parts) == 2 else 20
        print(engine.show_log(n))
        return

    raise SyntaxError(f"unknown command: {line!r}")

def repl() -> None:
    engine = Engine()
    print("Spherepop Reduction Engine. Type 'help' for commands, 'quit' to exit.")
    while True:
        try:
            line = input("spherepop> ").strip()
        except EOFError:
            print()
            return
        if line in ("quit", "exit"):
            return
        try:
            run_repl_command(engine, line)
        except Exception as exc:
            print(f"error: {exc}")

# ---------------------------------------------------------------------------
# File runner
# ---------------------------------------------------------------------------

def run_file(path: str) -> None:
    with open(path) as f:
        src = f.read()
    engine = Engine()
    # Top-level statements separated by blank lines become separate bubbles,
    # or the whole file is one bubble named after the filename.
    name = path.rsplit("/", 1)[-1].rsplit(".", 1)[0]
    engine.spawn(name, src)
    engine.run()
    print()
    print(engine.process_table())
    print()
    print(engine.show_log())

# ---------------------------------------------------------------------------
# Demo
# ---------------------------------------------------------------------------

def demo() -> None:
    print("=" * 60)
    print("Demo 1: Simple POP chain")
    print("=" * 60)
    e = Engine()
    e.spawn("counter", "set x 0; add x 1; add x 1; add x 1; print x; halt")
    e.run()
    print(e.show_log())

    print()
    print("=" * 60)
    print("Demo 2: REFUSE — sqrt of negative number")
    print("=" * 60)
    e = Engine()
    e.spawn("bad_math", "set x -9; sqrt x result; halt")
    e.run()
    print(e.show_log())

    print()
    print("=" * 60)
    print("Demo 3: REFUSE — division by zero")
    print("=" * 60)
    e = Engine()
    e.spawn("div_zero", "set x 10; div x 0; halt")
    e.run()
    print(e.show_log())

    print()
    print("=" * 60)
    print("Demo 4: COLLAPSE — value escapes upward to kernel")
    print("=" * 60)
    e = Engine()
    e.spawn("worker", "set x 6; mul x 7; collapse x")
    e.run()
    print(e.show_log())
    print(f"kernel inbox after run: {list(e.bubbles[1].inbox) if 1 in e.bubbles else '(kernel has no inbox)'}")

    print()
    print("=" * 60)
    print("Demo 5: Message passing — sender unblocks receiver")
    print("=" * 60)
    e = Engine()
    recv_id = e.spawn("receiver", "recv msg; print msg; collapse msg")
    send_id = e.spawn("sender",   f"send {recv_id} 42; halt")
    print(f"spawned receiver id={recv_id}, sender id={send_id}")
    e.run()
    print(e.show_log())
    print(e.process_table())

    print()
    print("=" * 60)
    print("Demo 6: Nested bubbles — orchestrator spawns a child")
    print("=" * 60)
    e = Engine()
    # The orchestrator spawns a child, then waits for the child to collapse a value
    e.spawn("orchestrator",
        "set n 10; add n 5; collapse n"
    )
    e.run()
    print(e.show_log())

# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

def main(argv: list[str]) -> int:
    if len(argv) > 1:
        if argv[1] == "--demo":
            demo()
        else:
            run_file(argv[1])
    else:
        repl()
    return 0

if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
