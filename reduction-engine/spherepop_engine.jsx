import { useState, useCallback, useRef, useEffect } from "react";

// ─── Outcome constants ──────────────────────────────────────────────
const Outcome = { POP: "POP", REFUSE: "REFUSE", COLLAPSE: "COLLAPSE" };
const BubbleState = { OPEN: "OPEN", POPPED: "POPPED", REFUSED: "REFUSED", COLLAPSED: "COLLAPSED" };

// ─── Instruction parser ─────────────────────────────────────────────
function parseProgram(src) {
  return src.split(";").map(s => s.trim()).filter(Boolean).map(raw => {
    const parts = raw.match(/"[^"]*"|\S+/g) || [];
    return { op: parts[0] || "", args: parts.slice(1), raw };
  });
}

// ─── Admissibility gate ─────────────────────────────────────────────
// This is where constraint checking lives. Returns Outcome.
function admissibilityGate(bubble, instr, engine) {
  const { op, args } = instr;

  // sqrt of negative = REFUSE
  if (op === "sqrt") {
    const val = resolveValue(bubble, args[0]);
    if (typeof val === "number" && val < 0) return Outcome.REFUSE;
  }

  // div by zero = REFUSE
  if (op === "div") {
    const divisor = resolveValue(bubble, args[1]);
    if (Number(divisor) === 0) return Outcome.REFUSE;
  }

  // recv on empty inbox when no sibling can unblock us = REFUSE
  if (op === "recv" && bubble.inbox.length === 0) {
    // check if any sibling or parent has a pending send targeting us
    const hasSender = Object.values(engine.bubbles).some(b =>
      b.state === BubbleState.OPEN &&
      b.program.slice(b.pc).some(i => i.op === "send" && i.args[0] === String(bubble.id))
    );
    if (!hasSender) return Outcome.REFUSE;
    return null; // block (stay OPEN, do not advance)
  }

  return Outcome.POP; // default: admissible reduction
}

function resolveValue(bubble, token) {
  if (token === undefined) return undefined;
  if (token in bubble.memory) return bubble.memory[token];
  if (token.startsWith('"') && token.endsWith('"')) return token.slice(1, -1);
  const n = Number(token);
  return isNaN(n) ? token : n;
}

// ─── Engine ─────────────────────────────────────────────────────────
function createEngine() {
  return {
    bubbles: {},       // id -> bubble
    nextId: 1,
    clock: 0,
    log: [],           // reduction log entries
  };
}

function cloneEngine(engine) {
  const b = {};
  for (const [k, v] of Object.entries(engine.bubbles)) {
    b[k] = {
      ...v,
      inbox: [...v.inbox],
      memory: { ...v.memory },
      labels: { ...v.labels },
      program: v.program, // immutable refs ok
      children: [...v.children],
    };
  }
  return { ...engine, bubbles: b, log: [...engine.log] };
}

function spawnBubble(engine, name, src, parentId = null) {
  const id = engine.nextId++;
  const program = parseProgram(src);
  const labels = {};
  program.forEach((instr, i) => {
    if (instr.op === "label" && instr.args[0]) labels[instr.args[0]] = i;
  });

  engine.bubbles[id] = {
    id, name, parentId,
    children: [],
    inbox: [],
    memory: {},
    labels,
    program,
    pc: 0,
    state: BubbleState.OPEN,
    collapsedValue: undefined,
  };

  if (parentId !== null && engine.bubbles[parentId]) {
    engine.bubbles[parentId].children.push(id);
  }

  return id;
}

// Single reduction step. Returns a log entry describing what happened.
function stepEngine(engine) {
  engine.clock++;

  // Find first OPEN bubble with a ready instruction (BFS by id)
  const candidates = Object.values(engine.bubbles)
    .filter(b => b.state === BubbleState.OPEN)
    .sort((a, b) => a.id - b.id);

  for (const bubble of candidates) {
    if (bubble.pc >= bubble.program.length) {
      bubble.state = BubbleState.POPPED;
      const entry = { clock: engine.clock, bubbleId: bubble.id, name: bubble.name, outcome: Outcome.POP, instr: "(end of program)", detail: "program exhausted → POP" };
      engine.log.push(entry);
      return entry;
    }

    const instr = bubble.program[bubble.pc];

    // Skip labels silently
    if (instr.op === "label") { bubble.pc++; continue; }

    // Admissibility check
    const gate = admissibilityGate(bubble, instr, engine);

    if (gate === Outcome.REFUSE) {
      bubble.state = BubbleState.REFUSED;
      const entry = { clock: engine.clock, bubbleId: bubble.id, name: bubble.name, outcome: Outcome.REFUSE, instr: instr.raw, detail: `constraint violation → REFUSE` };
      engine.log.push(entry);
      return entry;
    }

    if (gate === null) {
      // blocked, try next bubble
      continue;
    }

    // Execute instruction
    bubble.pc++;
    const result = executeInstruction(bubble, instr, engine);

    const entry = {
      clock: engine.clock,
      bubbleId: bubble.id,
      name: bubble.name,
      outcome: result.outcome,
      instr: instr.raw,
      detail: result.detail,
      value: result.value,
    };
    engine.log.push(entry);

    // Propagate collapse upward
    if (result.outcome === Outcome.COLLAPSE && bubble.parentId !== null) {
      const parent = engine.bubbles[bubble.parentId];
      if (parent && parent.state === BubbleState.OPEN) {
        parent.inbox.push(result.value);
        entry.detail += ` → inbox of bubble ${bubble.parentId}`;
      }
    }

    return entry;
  }

  return null; // no work done
}

function executeInstruction(bubble, instr, engine) {
  const { op, args } = instr;
  const rv = (t) => resolveValue(bubble, t);

  if (op === "halt") {
    bubble.state = BubbleState.POPPED;
    return { outcome: Outcome.POP, detail: "halt → POP" };
  }

  if (op === "collapse") {
    const val = rv(args[0]);
    bubble.state = BubbleState.COLLAPSED;
    bubble.collapsedValue = val;
    return { outcome: Outcome.COLLAPSE, detail: `collapse ${val}`, value: val };
  }

  if (op === "yield") {
    return { outcome: Outcome.POP, detail: "yield (cooperative)" };
  }

  if (op === "print") {
    const val = rv(args[0]);
    return { outcome: Outcome.POP, detail: `print → ${val}`, value: val };
  }

  if (op === "set") {
    bubble.memory[args[0]] = rv(args[1]);
    return { outcome: Outcome.POP, detail: `set ${args[0]} = ${bubble.memory[args[0]]}` };
  }

  if (op === "add") {
    const old = Number(bubble.memory[args[0]] ?? 0);
    bubble.memory[args[0]] = old + Number(rv(args[1]));
    return { outcome: Outcome.POP, detail: `add ${args[0]}: ${old} + ${rv(args[1])} = ${bubble.memory[args[0]]}` };
  }

  if (op === "mul") {
    const old = Number(bubble.memory[args[0]] ?? 1);
    bubble.memory[args[0]] = old * Number(rv(args[1]));
    return { outcome: Outcome.POP, detail: `mul ${args[0]}: ${old} × ${rv(args[1])} = ${bubble.memory[args[0]]}` };
  }

  if (op === "div") {
    const num = Number(bubble.memory[args[0]] ?? 0);
    const den = Number(rv(args[1]));
    bubble.memory[args[0]] = num / den;
    return { outcome: Outcome.POP, detail: `div ${args[0]}: ${num} / ${den} = ${bubble.memory[args[0]]}` };
  }

  if (op === "sqrt") {
    const val = Number(rv(args[0]));
    bubble.memory[args[1] || args[0]] = Math.sqrt(val);
    return { outcome: Outcome.POP, detail: `sqrt(${val}) = ${Math.sqrt(val)}` };
  }

  if (op === "send") {
    const targetId = Number(rv(args[0]));
    const val = rv(args[1]);
    const target = engine.bubbles[targetId];
    if (!target) return { outcome: Outcome.REFUSE, detail: `send: no bubble ${targetId} → REFUSE` };
    target.inbox.push(val);
    return { outcome: Outcome.POP, detail: `send ${val} → bubble ${targetId}` };
  }

  if (op === "recv") {
    if (bubble.inbox.length > 0) {
      const val = bubble.inbox.shift();
      bubble.memory[args[0]] = val;
      return { outcome: Outcome.POP, detail: `recv ${args[0]} ← ${val}` };
    }
    bubble.pc--; // retry
    return { outcome: Outcome.POP, detail: `recv blocked (inbox empty)` };
  }

  if (op === "jnz") {
    const val = Number(rv(args[0]));
    if (val !== 0) {
      const label = args[1];
      if (!(label in bubble.labels)) {
        bubble.state = BubbleState.REFUSED;
        return { outcome: Outcome.REFUSE, detail: `jnz: unknown label ${label} → REFUSE` };
      }
      bubble.pc = bubble.labels[label];
    }
    return { outcome: Outcome.POP, detail: `jnz ${args[0]}=${val} → ${val !== 0 ? args[1] : "no jump"}` };
  }

  if (op === "spawn") {
    // spawn NAME { ... } as a child bubble
    const name = args[0];
    const body = args.slice(1).join(" ").replace(/^\{/, "").replace(/\}$/, "").trim();
    const childId = spawnBubble(engine, name, body, bubble.id);
    bubble.memory[name] = childId; // store child id for send
    return { outcome: Outcome.POP, detail: `spawn child bubble ${childId} (${name})` };
  }

  return { outcome: Outcome.REFUSE, detail: `unknown op: ${op} → REFUSE` };
}

function isTerminated(engine) {
  return Object.values(engine.bubbles).every(b => b.state !== BubbleState.OPEN);
}

// ─── Preset programs ────────────────────────────────────────────────
const PRESETS = [
  {
    label: "Simple POP chain",
    name: "counter",
    src: `set x 0; add x 1; add x 1; add x 1; print x; halt`,
    description: "Each instruction reduces (POPs). Counter increments to 3.",
  },
  {
    label: "REFUSE: sqrt(-1)",
    name: "bad_math",
    src: `set x -4; sqrt x result; halt`,
    description: "sqrt of negative triggers admissibility gate → REFUSE. Bubble closes without crashing.",
  },
  {
    label: "REFUSE: div by zero",
    name: "div_zero",
    src: `set x 10; div x 0; halt`,
    description: "Division by zero is caught by the admissibility gate → REFUSE.",
  },
  {
    label: "COLLAPSE to parent",
    name: "parent",
    src: `set x 6; mul x 7; collapse x`,
    description: "Bubble computes 6×7=42 then COLLAPSEs the result upward into the kernel inbox.",
  },
  {
    label: "Nested: spawn + collapse",
    name: "orchestrator",
    src: `set n 5; add n 5; collapse n`,
    description: "Orchestrator computes 5+5=10 and collapses it to kernel. Shows how nested computation returns values.",
  },
  {
    label: "Message passing",
    name: "receiver",
    src: `recv msg; print msg; collapse msg`,
    extraBubbles: [
      { name: "sender", src: `send 1 99; halt` }
    ],
    description: "Sender sends 99 to receiver (pid 1). Receiver recvs, prints, then collapses value upward.",
  },
];

// ─── UI ─────────────────────────────────────────────────────────────
const outcomeColor = {
  POP: "#4ade80",
  REFUSE: "#f87171",
  COLLAPSE: "#a78bfa",
};

const stateColor = {
  OPEN: "#60a5fa",
  POPPED: "#4ade80",
  REFUSED: "#f87171",
  COLLAPSED: "#a78bfa",
};

export default function SpherepoEngine() {
  const [engine, setEngine] = useState(() => createEngine());
  const [running, setRunning] = useState(false);
  const [speed, setSpeed] = useState(400);
  const [customSrc, setCustomSrc] = useState("");
  const [customName, setCustomName] = useState("bubble");
  const [activePreset, setActivePreset] = useState(null);
  const intervalRef = useRef(null);

  const loadPreset = useCallback((preset) => {
    const eng = createEngine();
    spawnBubble(eng, preset.name, preset.src, null);
    if (preset.extraBubbles) {
      preset.extraBubbles.forEach(b => spawnBubble(eng, b.name, b.src, null));
    }
    setEngine(eng);
    setRunning(false);
    setActivePreset(preset);
  }, []);

  const spawnCustom = useCallback(() => {
    if (!customSrc.trim()) return;
    const eng = cloneEngine(engine);
    spawnBubble(eng, customName || "bubble", customSrc, null);
    setEngine(eng);
    setCustomSrc("");
    setActivePreset(null);
  }, [engine, customSrc, customName]);

  const step = useCallback(() => {
    setEngine(prev => {
      if (isTerminated(prev)) return prev;
      const eng = cloneEngine(prev);
      stepEngine(eng);
      return eng;
    });
  }, []);

  const reset = useCallback(() => {
    if (activePreset) loadPreset(activePreset);
    else setEngine(createEngine());
    setRunning(false);
  }, [activePreset, loadPreset]);

  useEffect(() => {
    if (running) {
      intervalRef.current = setInterval(() => {
        setEngine(prev => {
          if (isTerminated(prev)) {
            setRunning(false);
            return prev;
          }
          const eng = cloneEngine(prev);
          const result = stepEngine(eng);
          if (!result) setRunning(false);
          return eng;
        });
      }, speed);
    }
    return () => clearInterval(intervalRef.current);
  }, [running, speed]);

  const bubbles = Object.values(engine.bubbles).sort((a, b) => a.id - b.id);
  const terminated = isTerminated(engine);
  const log = [...engine.log].reverse().slice(0, 60);

  return (
    <div style={{
      fontFamily: "'JetBrains Mono', 'Fira Code', monospace",
      background: "#0a0a0f",
      color: "#c9d1d9",
      minHeight: "100vh",
      display: "flex",
      flexDirection: "column",
      padding: "0",
    }}>
      {/* Header */}
      <div style={{
        borderBottom: "1px solid #1e2030",
        padding: "18px 28px",
        display: "flex",
        alignItems: "baseline",
        gap: "20px",
        background: "#0d0d14",
      }}>
        <div style={{ fontSize: "18px", fontWeight: 700, color: "#e2e8f0", letterSpacing: "0.04em" }}>
          ◉ SPHEREPOP
        </div>
        <div style={{ fontSize: "11px", color: "#555", letterSpacing: "0.12em", textTransform: "uppercase" }}>
          Reduction Engine · Pop / Refuse / Collapse
        </div>
        <div style={{ marginLeft: "auto", fontSize: "11px", color: "#444" }}>
          clock={engine.clock} &nbsp;|&nbsp; bubbles={bubbles.length}
        </div>
      </div>

      <div style={{ display: "flex", flex: 1, overflow: "hidden" }}>
        {/* Left panel */}
        <div style={{ width: "300px", borderRight: "1px solid #1e2030", display: "flex", flexDirection: "column", background: "#0b0b12" }}>
          {/* Presets */}
          <div style={{ padding: "16px", borderBottom: "1px solid #1a1a24" }}>
            <div style={{ fontSize: "10px", color: "#444", letterSpacing: "0.14em", marginBottom: "10px", textTransform: "uppercase" }}>Presets</div>
            {PRESETS.map((p, i) => (
              <button key={i} onClick={() => loadPreset(p)} style={{
                display: "block", width: "100%", textAlign: "left",
                background: activePreset === p ? "#1a1a2e" : "transparent",
                border: activePreset === p ? "1px solid #2d2d4a" : "1px solid transparent",
                color: activePreset === p ? "#a78bfa" : "#888",
                padding: "7px 10px", marginBottom: "3px", borderRadius: "4px",
                cursor: "pointer", fontSize: "11px",
                transition: "all 0.15s",
              }}>
                {p.label}
              </button>
            ))}
          </div>

          {/* Description */}
          {activePreset && (
            <div style={{ padding: "12px 16px", borderBottom: "1px solid #1a1a24" }}>
              <div style={{ fontSize: "10px", color: "#555", lineHeight: 1.6 }}>{activePreset.description}</div>
            </div>
          )}

          {/* Custom spawn */}
          <div style={{ padding: "16px", borderBottom: "1px solid #1a1a24" }}>
            <div style={{ fontSize: "10px", color: "#444", letterSpacing: "0.14em", marginBottom: "10px", textTransform: "uppercase" }}>Spawn bubble</div>
            <input
              value={customName}
              onChange={e => setCustomName(e.target.value)}
              placeholder="name"
              style={{ width: "100%", boxSizing: "border-box", background: "#111118", border: "1px solid #222", color: "#aaa", padding: "6px 8px", borderRadius: "3px", fontSize: "11px", marginBottom: "6px" }}
            />
            <textarea
              value={customSrc}
              onChange={e => setCustomSrc(e.target.value)}
              placeholder={"set x 10; div x 0; halt"}
              rows={4}
              style={{ width: "100%", boxSizing: "border-box", background: "#111118", border: "1px solid #222", color: "#aaa", padding: "6px 8px", borderRadius: "3px", fontSize: "11px", resize: "vertical", fontFamily: "inherit" }}
            />
            <button onClick={spawnCustom} style={{
              width: "100%", marginTop: "6px", padding: "7px",
              background: "#1a1a2e", border: "1px solid #2d2d4a",
              color: "#a78bfa", borderRadius: "3px", cursor: "pointer", fontSize: "11px",
            }}>
              ⊕ Spawn
            </button>
          </div>

          {/* Controls */}
          <div style={{ padding: "16px" }}>
            <div style={{ fontSize: "10px", color: "#444", letterSpacing: "0.14em", marginBottom: "10px", textTransform: "uppercase" }}>Controls</div>
            <div style={{ display: "flex", gap: "6px", marginBottom: "10px" }}>
              <button onClick={step} disabled={terminated || running} style={btnStyle("#1a2a1a", "#4ade80", terminated || running)}>
                ▶ Step
              </button>
              <button onClick={() => setRunning(r => !r)} disabled={terminated} style={btnStyle("#1a1a2e", "#a78bfa", terminated)}>
                {running ? "⏸ Pause" : "⏩ Run"}
              </button>
              <button onClick={reset} style={btnStyle("#2a1a1a", "#f87171", false)}>
                ↺
              </button>
            </div>
            <div style={{ display: "flex", alignItems: "center", gap: "8px", fontSize: "10px", color: "#555" }}>
              <span>Speed</span>
              <input type="range" min={50} max={1000} step={50} value={speed}
                onChange={e => setSpeed(Number(e.target.value))}
                style={{ flex: 1, accentColor: "#a78bfa" }} />
              <span>{speed}ms</span>
            </div>
          </div>

          {/* Legend */}
          <div style={{ padding: "16px", marginTop: "auto", borderTop: "1px solid #1a1a24" }}>
            {Object.entries(outcomeColor).map(([k, c]) => (
              <div key={k} style={{ display: "flex", alignItems: "center", gap: "8px", marginBottom: "5px", fontSize: "10px" }}>
                <span style={{ width: 8, height: 8, borderRadius: "50%", background: c, display: "inline-block" }} />
                <span style={{ color: c, fontWeight: 600 }}>{k}</span>
                <span style={{ color: "#444" }}>
                  {k === "POP" ? "successful reduction" : k === "REFUSE" ? "constraint violation" : "value escapes upward"}
                </span>
              </div>
            ))}
          </div>
        </div>

        {/* Main area */}
        <div style={{ flex: 1, display: "flex", flexDirection: "column", overflow: "hidden" }}>
          {/* Bubble tree */}
          <div style={{ padding: "20px 24px", borderBottom: "1px solid #1e2030", flex: "0 0 auto" }}>
            <div style={{ fontSize: "10px", color: "#444", letterSpacing: "0.14em", marginBottom: "14px", textTransform: "uppercase" }}>
              Bubble Hierarchy
            </div>
            {bubbles.length === 0 ? (
              <div style={{ color: "#333", fontSize: "12px" }}>kernel() — no bubbles spawned</div>
            ) : (
              <div style={{ display: "flex", flexWrap: "wrap", gap: "10px" }}>
                {bubbles.map(b => <BubbleCard key={b.id} bubble={b} />)}
              </div>
            )}
          </div>

          {/* Reduction log */}
          <div style={{ flex: 1, overflow: "auto", padding: "20px 24px" }}>
            <div style={{ fontSize: "10px", color: "#444", letterSpacing: "0.14em", marginBottom: "14px", textTransform: "uppercase" }}>
              Reduction Log <span style={{ color: "#333" }}>({engine.log.length} steps)</span>
            </div>
            {log.length === 0 && (
              <div style={{ color: "#333", fontSize: "12px" }}>No reductions yet. Load a preset or spawn a bubble, then Step or Run.</div>
            )}
            {log.map((entry, i) => (
              <LogEntry key={i} entry={entry} />
            ))}
          </div>
        </div>
      </div>
    </div>
  );
}

function BubbleCard({ bubble }) {
  const sc = stateColor[bubble.state];
  const memEntries = Object.entries(bubble.memory);
  return (
    <div style={{
      border: `1px solid ${sc}33`,
      borderRadius: "6px",
      padding: "12px 14px",
      background: `${sc}08`,
      minWidth: "180px",
      maxWidth: "240px",
      position: "relative",
    }}>
      <div style={{ display: "flex", alignItems: "center", gap: "6px", marginBottom: "8px" }}>
        <span style={{ width: 7, height: 7, borderRadius: "50%", background: sc, display: "inline-block" }} />
        <span style={{ color: "#e2e8f0", fontSize: "12px", fontWeight: 600 }}>
          [{bubble.id}] {bubble.name}
        </span>
      </div>
      <div style={{ fontSize: "10px", color: sc, marginBottom: "6px", letterSpacing: "0.1em" }}>
        {bubble.state}
      </div>
      <div style={{ fontSize: "10px", color: "#555", marginBottom: "4px" }}>
        pc={bubble.pc}/{bubble.program.length} &nbsp; inbox={bubble.inbox.length}
      </div>
      {bubble.collapsedValue !== undefined && (
        <div style={{ fontSize: "10px", color: "#a78bfa", marginTop: "4px" }}>
          ↑ collapsed: {String(bubble.collapsedValue)}
        </div>
      )}
      {memEntries.length > 0 && (
        <div style={{ marginTop: "8px", borderTop: "1px solid #1a1a24", paddingTop: "6px" }}>
          {memEntries.slice(0, 4).map(([k, v]) => (
            <div key={k} style={{ fontSize: "10px", color: "#666" }}>
              {k} = <span style={{ color: "#aaa" }}>{String(v)}</span>
            </div>
          ))}
          {memEntries.length > 4 && <div style={{ fontSize: "10px", color: "#444" }}>+{memEntries.length - 4} more</div>}
        </div>
      )}
      {bubble.program.length > 0 && bubble.state === BubbleState.OPEN && bubble.pc < bubble.program.length && (
        <div style={{ marginTop: "8px", borderTop: "1px solid #1a1a24", paddingTop: "6px" }}>
          <div style={{ fontSize: "10px", color: "#555" }}>next:</div>
          <div style={{ fontSize: "10px", color: "#7c8db0" }}>{bubble.program[bubble.pc]?.raw}</div>
        </div>
      )}
    </div>
  );
}

function LogEntry({ entry }) {
  const c = outcomeColor[entry.outcome] || "#888";
  return (
    <div style={{
      display: "flex", alignItems: "baseline", gap: "10px",
      padding: "4px 0",
      borderBottom: "1px solid #111118",
      fontSize: "11px",
    }}>
      <span style={{ color: "#333", width: "36px", textAlign: "right", flexShrink: 0 }}>
        {entry.clock}
      </span>
      <span style={{ color: "#3a3a4a", width: "14px", flexShrink: 0 }}>│</span>
      <span style={{ color: "#555", width: "60px", flexShrink: 0 }}>
        [{entry.bubbleId}:{entry.name.slice(0,6)}]
      </span>
      <span style={{
        width: "68px", flexShrink: 0,
        color: c, fontWeight: 600, letterSpacing: "0.06em",
      }}>
        {entry.outcome}
      </span>
      <span style={{ color: "#666", flex: 1, overflow: "hidden", whiteSpace: "nowrap", textOverflow: "ellipsis" }}>
        {entry.detail}
      </span>
    </div>
  );
}

function btnStyle(bg, col, disabled) {
  return {
    background: disabled ? "#111" : bg,
    border: `1px solid ${disabled ? "#222" : col + "55"}`,
    color: disabled ? "#333" : col,
    padding: "6px 12px",
    borderRadius: "3px",
    cursor: disabled ? "not-allowed" : "pointer",
    fontSize: "11px",
    fontFamily: "inherit",
    flex: 1,
  };
}
