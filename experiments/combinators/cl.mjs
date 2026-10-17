// Application circles over the constants S and K, hash-consed.
// A term is an atom (S, K, or a free variable) or a binary application circle A(f, x).
// Interning makes identical subterms the same object, so sharing is automatic and
// equality of terms is identity of nodes.

export class BudgetError extends Error {
  constructor(message, info = {}) {
    super(message);
    this.info = info;
  }
}

const table = new Map();
let counter = 0;

function atomNode(name) {
  const key = "a" + name;
  let n = table.get(key);
  if (!n) {
    n = {
      id: counter++,
      kind: 0,
      name,
      f: null,
      x: null,
      size: 0n,
      leaves: 1n,
      depth: 0,
      rk: 0,
      has: false,
    };
    table.set(key, n);
  }
  return n;
}

export const S = atomNode("S");
export const K = atomNode("K");

export const atom = (name) => atomNode(name);
export const isVar = (t) => t.kind === 0 && t !== S && t !== K;

export function app(f, x) {
  const key = f.id + ":" + x.id;
  let n = table.get(key);
  if (!n) {
    const rk = f.kind === 1 && f.f === K ? 1 : f.kind === 1 && f.f.kind === 1 && f.f.f === S ? 2 : 0;
    n = {
      id: counter++,
      kind: 1,
      name: null,
      f,
      x,
      size: f.size + x.size + 1n,
      leaves: f.leaves + x.leaves,
      depth: 1 + Math.max(f.depth, x.depth),
      rk,
      has: rk > 0 || f.has || x.has,
    };
    table.set(key, n);
  }
  return n;
}

// A(f, a, b, ...) is left-associated application.
export const A = (f, ...args) => args.reduce((acc, a) => app(acc, a), f);

export const I = A(S, K, K);
export const macros = { I };

export function spine(t) {
  const args = [];
  let h = t;
  while (h.kind === 1) {
    args.push(h.x);
    h = h.f;
  }
  args.reverse();
  return { head: h, args };
}

export function show(t, { macroI = true } = {}) {
  const sh = (u, wrap) => {
    if (macroI && u === I) return "I";
    if (u.kind === 0) return u.name;
    const { head, args } = spine(u);
    const parts = [sh(head, true), ...args.map((a) => sh(a, true))];
    const s = parts.join(" ");
    return wrap ? "(" + s + ")" : s;
  };
  return sh(t, false);
}

export function parse(src) {
  const toks = src.match(/[()]|[A-Za-z][A-Za-z0-9_']*/g) || [];
  let i = 0;
  function seq() {
    let acc = null;
    while (i < toks.length && toks[i] !== ")") {
      let u;
      if (toks[i] === "(") {
        i++;
        u = seq();
        if (toks[i] !== ")") throw new Error("Unbalanced parentheses");
        i++;
      } else {
        const name = toks[i++];
        u = macros[name] ?? atom(name);
      }
      acc = acc === null ? u : app(acc, u);
    }
    if (acc === null) throw new Error("Empty expression");
    return acc;
  }
  const t = seq();
  if (i !== toks.length) throw new Error("Unbalanced parentheses");
  return t;
}

// ---- measures ----------------------------------------------------------
export const size = (t) => t.size; // circles in the tree (BigInt)
export const leaves = (t) => t.leaves;
export function dagSize(t) {
  const seen = new Set();
  const stack = [t];
  while (stack.length) {
    const u = stack.pop();
    if (seen.has(u.id)) continue;
    seen.add(u.id);
    if (u.kind === 1) stack.push(u.f, u.x);
  }
  return seen.size;
}

// ---- free variables, substitution ------------------------------------------
const fvCache = new Map();
export function freeVars(t) {
  if (t.kind === 0) return isVar(t) ? new Set([t.name]) : new Set();
  let s = fvCache.get(t.id);
  if (!s) {
    s = new Set([...freeVars(t.f), ...freeVars(t.x)]);
    fvCache.set(t.id, s);
  }
  return s;
}
export const hasVar = (t, name) => freeVars(t).has(name);

export function subst(t, name, value, memo = new Map()) {
  if (t.kind === 0) return t.name === name && isVar(t) ? value : t;
  if (!hasVar(t, name)) return t;
  let r = memo.get(t.id);
  if (!r) {
    r = app(subst(t.f, name, value, memo), subst(t.x, name, value, memo));
    memo.set(t.id, r);
  }
  return r;
}

// ---- contraction ---------------------------------------------------------
export function contract(t) {
  if (t.rk === 1) return t.f.x;
  if (t.rk === 2) {
    const x = t.f.f.x;
    const y = t.f.x;
    const z = t.x;
    return app(app(x, z), app(y, z));
  }
  throw new Error("not a redex");
}

export const ruleOf = (t) => (t.rk === 1 ? "K" : t.rk === 2 ? "S" : null);

export function at(t, path) {
  let u = t;
  for (const p of path) u = p === 0 ? u.f : u.x;
  return u;
}
export function replaceAt(t, path, value, i = 0) {
  if (i === path.length) return value;
  return path[i] === 0
    ? app(replaceAt(t.f, path, value, i + 1), t.x)
    : app(t.f, replaceAt(t.x, path, value, i + 1));
}
export function contractAt(t, path) {
  const u = at(t, path);
  if (u.rk === 0) throw new Error("no redex at path " + path.join(""));
  return replaceAt(t, path, contract(u));
}

// All redex positions in tree order (pre-order, f before x). Exponential on shared
// terms, so only for small inputs.
export function redexPaths(t, path = [], out = []) {
  if (!t.has) return out;
  if (t.rk) out.push(path.slice());
  if (t.kind === 1) {
    redexPaths(t.f, [...path, 0], out);
    redexPaths(t.x, [...path, 1], out);
  }
  return out;
}

// ---- one-step strategies on trees ------------------------------------------
function stepLO(t) {
  if (!t.has) return null;
  if (t.rk) return contract(t);
  if (t.f.has) return app(stepLO(t.f), t.x);
  return app(t.f, stepLO(t.x));
}
function stepLI(t) {
  if (!t.has) return null;
  if (t.f !== null && t.f.has) return app(stepLI(t.f), t.x);
  if (t.x !== null && t.x.has) return app(t.f, stepLI(t.x));
  return contract(t);
}
function stepRI(t) {
  if (!t.has) return null;
  if (t.x !== null && t.x.has) return app(t.f, stepRI(t.x));
  if (t.f !== null && t.f.has) return app(stepRI(t.f), t.x);
  return contract(t);
}
function stepRO(t) {
  if (!t.has) return null;
  if (t.rk) return contract(t);
  if (t.x.has) return app(t.f, stepRO(t.x));
  return app(stepRO(t.f), t.x);
}
const steppers = { lo: stepLO, li: stepLI, ri: stepRI, ro: stepRO };
export const step = (t, strategy = "lo") => steppers[strategy](t);

// Gross-Knuth: contract every redex that is present, all at once. Memoized by node.
export function gk(t, memo = new Map()) {
  if (!t.has) return t;
  let r = memo.get(t.id);
  if (r) return r;
  if (t.rk === 1) r = gk(t.f.x, memo);
  else if (t.rk === 2) {
    const z = gk(t.x, memo);
    r = app(app(gk(t.f.f.x, memo), z), app(gk(t.f.x, memo), z));
  } else r = app(gk(t.f, memo), gk(t.x, memo));
  memo.set(t.id, r);
  return r;
}

// All single-step reducts, one entry per redex occurrence in the tree.
const reductsCache = new Map();
export function reducts(t) {
  if (!t.has) return [];
  let r = reductsCache.get(t.id);
  if (r) return r;
  r = [];
  if (t.rk) r.push(contract(t));
  if (t.kind === 1) {
    if (t.f.has) for (const u of reducts(t.f)) r.push(app(u, t.x));
    if (t.x.has) for (const u of reducts(t.x)) r.push(app(t.f, u));
  }
  reductsCache.set(t.id, r);
  return r;
}
export const clearCaches = () => reductsCache.clear();

// The parallel reduction relation of Tait and Martin-Lof: any set of redexes may be
// contracted simultaneously, including redexes nested inside arguments.
const prCache = new Map();
export function parReducts(t) {
  if (t.kind === 0) return new Map([[t.id, t]]);
  let r = prCache.get(t.id);
  if (r) return r;
  r = new Map();
  const F = parReducts(t.f);
  const X = parReducts(t.x);
  for (const f of F.values()) for (const x of X.values()) {
    const u = app(f, x);
    r.set(u.id, u);
  }
  if (t.rk === 1) for (const a of parReducts(t.f.x).values()) r.set(a.id, a);
  if (t.rk === 2) {
    const X1 = parReducts(t.f.f.x);
    const Y1 = parReducts(t.f.x);
    const Z1 = parReducts(t.x);
    for (const x of X1.values()) for (const y of Y1.values()) for (const z of Z1.values()) {
      const u = app(app(x, z), app(y, z));
      r.set(u.id, u);
    }
  }
  prCache.set(t.id, r);
  return r;
}

// ---- normalization by a chosen strategy, with cycle detection -----------------
export function normalize(t, strategy = "lo", { maxSteps = 100000, maxSize = 10n ** 9n, trace = false } = {}) {
  const seen = new Map([[t.id, 0]]);
  const history = trace ? [t] : null;
  let steps = 0;
  for (;;) {
    const next = step(t, strategy);
    if (next === null) return { status: "normal", term: t, steps, trace: history };
    t = next;
    steps++;
    if (history) history.push(t);
    if (seen.has(t.id)) return { status: "cycle", term: t, steps, period: steps - seen.get(t.id), trace: history };
    seen.set(t.id, steps);
    if (steps >= maxSteps) return { status: "budget", term: t, steps, trace: history };
    if (t.size > maxSize) return { status: "size", term: t, steps, trace: history };
  }
}

export function gkNormalize(t, { maxSteps = 10000, maxSize = 10n ** 9n } = {}) {
  const seen = new Set([t.id]);
  let rounds = 0;
  while (t.has) {
    t = gk(t);
    rounds++;
    if (seen.has(t.id)) return { status: "cycle", term: t, rounds };
    seen.add(t.id);
    if (rounds >= maxSteps) return { status: "budget", term: t, rounds };
    if (t.size > maxSize) return { status: "size", term: t, rounds };
  }
  return { status: "normal", term: t, rounds };
}

// ---- leftmost-outermost normal form by an abstract machine --------------------
// Head reduction with an argument stack, then the arguments in order. With memo=false
// this performs exactly the contractions of the tree strategy "lo"; with memo=true a
// shared subterm is normalized once (graph reduction).
export function whnf(t, ctx) {
  const stack = [];
  let cur = t;
  for (;;) {
    while (cur.kind === 1) {
      stack.push(cur.x);
      cur = cur.f;
    }
    if (cur === K && stack.length >= 2) {
      const a = stack.pop();
      stack.pop();
      cur = a;
    } else if (cur === S && stack.length >= 3) {
      const x = stack.pop();
      const y = stack.pop();
      const z = stack.pop();
      cur = app(app(x, z), app(y, z));
    } else break;
    if (++ctx.steps > ctx.max) throw new BudgetError("step budget exhausted", { steps: ctx.steps });
  }
  return { head: cur, stack };
}

function nf(t, ctx) {
  if (!t.has) return t;
  if (ctx.memo) {
    const c = ctx.memo.get(t.id);
    if (c) return c;
  }
  const { head, stack } = whnf(t, ctx);
  let out = head;
  for (let i = stack.length - 1; i >= 0; i--) out = app(out, nf(stack[i], ctx));
  if (ctx.memo) ctx.memo.set(t.id, out);
  return out;
}

export function nfLO(t, { maxSteps = 5e6, memo = false } = {}) {
  const ctx = { steps: 0, max: maxSteps, memo: memo ? new Map() : null };
  try {
    const term = nf(t, ctx);
    return { status: "normal", term, steps: ctx.steps };
  } catch (e) {
    if (e instanceof BudgetError) return { status: "budget", term: null, steps: ctx.steps };
    if (e instanceof RangeError) return { status: "stack", term: null, steps: ctx.steps };
    throw e;
  }
}


// Graph reduction: head normal forms of shared subterms are remembered by node id, so a
// term that is shared in the DAG is reduced at most once (Wadsworth's sharing, expressed
// as a memo table over interned nodes).  The recursion depth is bounded by the nesting of
// head evaluations, so this is used for sharing experiments, not for long machine runs.
function whnfShared(t, ctx) {
  if (t.kind === 0) return { head: t, stack: [] };
  const hit = ctx.wmemo.get(t.id);
  if (hit) return hit;
  const stack = [];
  let cur = t;
  for (;;) {
    while (cur.kind === 1) {
      const h = cur !== t ? ctx.wmemo.get(cur.id) : null;
      if (h) {
        for (const a of h.stack) stack.push(a);
        cur = h.head;
        break;
      }
      stack.push(cur.x);
      cur = cur.f;
    }
    if (cur === K && stack.length >= 2) {
      const a = stack.pop();
      stack.pop();
      if (++ctx.steps > ctx.max) throw new BudgetError("step budget exhausted", { steps: ctx.steps });
      const r = whnfShared(a, ctx);
      for (const x of r.stack) stack.push(x);
      cur = r.head;
    } else if (cur === S && stack.length >= 3) {
      const x = stack.pop();
      const y = stack.pop();
      const z = stack.pop();
      if (++ctx.steps > ctx.max) throw new BudgetError("step budget exhausted", { steps: ctx.steps });
      stack.push(app(y, z));
      const r = whnfShared(app(x, z), ctx);
      for (const a of r.stack) stack.push(a);
      cur = r.head;
    } else break;
  }
  const res = { head: cur, stack: stack.slice() };
  ctx.wmemo.set(t.id, res);
  return res;
}

function nfSharedRec(t, ctx) {
  if (!t.has) return t;
  const c = ctx.nmemo.get(t.id);
  if (c) return c;
  const { head, stack } = whnfShared(t, ctx);
  let out = head;
  for (let i = stack.length - 1; i >= 0; i--) out = app(out, nfSharedRec(stack[i], ctx));
  ctx.nmemo.set(t.id, out);
  return out;
}

export function nfShared(t, { maxSteps = 5e6 } = {}) {
  const ctx = { steps: 0, max: maxSteps, wmemo: new Map(), nmemo: new Map() };
  try {
    return { status: "normal", term: nfSharedRec(t, ctx), steps: ctx.steps };
  } catch (e) {
    if (e instanceof BudgetError) return { status: "budget", term: null, steps: ctx.steps };
    if (e instanceof RangeError) return { status: "stack", term: null, steps: ctx.steps };
    throw e;
  }
}

// ---- bracket abstraction ------------------------------------------------------
// Plain algorithm: [x]M = K M if x is not free in M; [x]x = I; [x](P Q) = S ([x]P) ([x]Q).
// With eta=true, [x](P x) = P is used when x is not free in P.
export function abstractVar(name, M, { eta = false } = {}) {
  const memo = new Map();
  const go = (m) => {
    if (!hasVar(m, name)) return app(K, m);
    if (m.kind === 0) return I; // m is the variable itself
    let r = memo.get(m.id);
    if (r) return r;
    if (eta && m.x.kind === 0 && m.x.name === name && isVar(m.x) && !hasVar(m.f, name)) r = m.f;
    else r = app(app(S, go(m.f)), go(m.x));
    memo.set(m.id, r);
    return r;
  };
  return go(M);
}

// Number of contractions in the designated reduction of ([x]M) N to M[x:=N]
// (plain algorithm).
export function abstractionSteps(name, M) {
  if (!hasVar(M, name)) return 1;
  if (M.kind === 0) return 2;
  return 1 + abstractionSteps(name, M.f) + abstractionSteps(name, M.x);
}

// The designated reduction of ([x]M) N, written out contraction by contraction, each
// one validated as a redex at an explicit position.
export function deriveAbstraction(name, M, N) {
  let cur = app(abstractVar(name, M), N);
  const log = [];
  const fire = (path, expect) => {
    const u = at(cur, path);
    if (ruleOf(u) !== expect) throw new Error(`expected rule ${expect} at ${path.join("")}`);
    cur = contractAt(cur, path);
    log.push({ path: path.join(""), rule: expect });
  };
  const go = (m, path) => {
    if (!hasVar(m, name)) return fire(path, "K");
    if (m.kind === 0) {
      fire(path, "S");
      return fire(path, "K");
    }
    fire(path, "S");
    go(m.f, [...path, 0]);
    go(m.x, [...path, 1]);
  };
  go(M, []);
  return { final: cur, log };
}

// ---- enumeration -----------------------------------------------------------------
export function* enumerate(n, atoms) {
  if (n === 1) {
    for (const a of atoms) yield a;
    return;
  }
  for (let k = 1; k < n; k++) {
    const rights = [...enumerate(n - k, atoms)];
    for (const l of enumerate(k, atoms)) for (const r of rights) yield app(l, r);
  }
}
