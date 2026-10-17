// Named lambda terms, capture-avoiding substitution, weak and full normal-order
// reduction, and the translation into S/K circles.
import * as C from "./cl.mjs";

export const v = (n) => ({ t: "v", n });
export const lam = (n, b) => ({ t: "l", n, b });
export const ap = (f, x) => ({ t: "a", f, x });
export const apps = (f, ...xs) => xs.reduce((a, x) => ap(a, x), f);
export const lams = (names, b) => names.reduceRight((acc, n) => lam(n, acc), b);

const fvMemo = new WeakMap();
function fvOf(m) {
  let r = fvMemo.get(m);
  if (r) return r;
  if (m.t === "v") r = new Set([m.n]);
  else if (m.t === "a") r = new Set([...fvOf(m.f), ...fvOf(m.x)]);
  else {
    r = new Set(fvOf(m.b));
    r.delete(m.n);
  }
  fvMemo.set(m, r);
  return r;
}
export function fv(m, acc = new Set()) {
  fvOf(m).forEach((x) => acc.add(x));
  return acc;
}

let freshCounter = 0;
const fresh = (base, avoid) => {
  let n;
  do n = base.replace(/_\d+$/, "") + "_" + ++freshCounter;
  while (avoid.has(n));
  return n;
};

export function subst(m, x, N) {
  if (!fvOf(m).has(x)) return m;
  if (m.t === "v") return N;
  if (m.t === "a") return ap(subst(m.f, x, N), subst(m.x, x, N));
  if (m.n === x) return m;
  const fn = fvOf(N);
  if (!fn.has(m.n)) return lam(m.n, subst(m.b, x, N));
  const avoid = new Set([...fn, ...fvOf(m.b), x]);
  const z = fresh(m.n, avoid);
  return lam(z, subst(subst(m.b, m.n, v(z)), x, N));
}

export function show(m) {
  if (m.t === "v") return m.n;
  if (m.t === "l") {
    const names = [];
    let b = m;
    while (b.t === "l") {
      names.push(b.n);
      b = b.b;
    }
    return "\\" + names.join(" ") + ". " + show(b);
  }
  const sh = (u, wrap) => (u.t === "v" ? u.n : wrap ? "(" + show(u) + ")" : show(u));
  const parts = [];
  let h = m;
  while (h.t === "a") {
    parts.push(h.x);
    h = h.f;
  }
  parts.reverse();
  return [sh(h, true), ...parts.map((p) => sh(p, true))].join(" ");
}

export function parse(src) {
  const toks = src.match(/[()\\.λ]|[A-Za-z][A-Za-z0-9_']*/g) || [];
  let i = 0;
  const atomStart = () => i < toks.length && toks[i] !== ")" && toks[i] !== ".";
  function seq() {
    if (toks[i] === "\\" || toks[i] === "λ") {
      i++;
      const names = [];
      while (toks[i] !== ".") names.push(toks[i++]);
      i++;
      return lams(names, seq());
    }
    let acc = null;
    while (atomStart()) {
      let u;
      if (toks[i] === "(") {
        i++;
        u = seq();
        if (toks[i++] !== ")") throw new Error("Unbalanced parentheses");
      } else if (toks[i] === "\\" || toks[i] === "λ") {
        u = seq();
      } else u = v(toks[i++]);
      acc = acc === null ? u : ap(acc, u);
    }
    if (acc === null) throw new Error("Empty expression");
    return acc;
  }
  const m = seq();
  if (i !== toks.length) throw new Error("Trailing input");
  return m;
}

// ---- reduction ---------------------------------------------------------------
// Weak: never reduce under a binder. Leftmost-outermost redex.
export function weakStep(m) {
  if (m.t === "l" || m.t === "v") return null;
  if (m.f.t === "l") return subst(m.f.b, m.f.n, m.x);
  const f = weakStep(m.f);
  if (f) return ap(f, m.x);
  const x = weakStep(m.x);
  return x ? ap(m.f, x) : null;
}
// Full normal order (reduces under binders).
export function fullStep(m) {
  if (m.t === "v") return null;
  if (m.t === "l") {
    const b = fullStep(m.b);
    return b ? lam(m.n, b) : null;
  }
  if (m.f.t === "l") return subst(m.f.b, m.f.n, m.x);
  const f = fullStep(m.f);
  if (f) return ap(f, m.x);
  const x = fullStep(m.x);
  return x ? ap(m.f, x) : null;
}
export function run(m, stepFn, maxSteps = 1e6) {
  let steps = 0;
  for (;;) {
    const n = stepFn(m);
    if (!n) return { status: "normal", term: m, steps };
    m = n;
    if (++steps >= maxSteps) return { status: "budget", term: m, steps };
  }
}

// alpha-equivalence
export function alphaEq(a, b, env = new Map(), rev = new Map()) {
  if (a.t !== b.t) return false;
  if (a.t === "v") {
    const ma = env.get(a.n);
    const mb = rev.get(b.n);
    if (ma === undefined && mb === undefined) return a.n === b.n;
    return ma === b.n && mb === a.n;
  }
  if (a.t === "a") return alphaEq(a.f, b.f, env, rev) && alphaEq(a.x, b.x, env, rev);
  const e2 = new Map(env).set(a.n, b.n);
  const r2 = new Map(rev).set(b.n, a.n);
  return alphaEq(a.b, b.b, e2, r2);
}

// ---- translation to circles ------------------------------------------------------
export function toCL(m, opts = {}) {
  if (m.t === "v") return C.atom(m.n);
  if (m.t === "a") return C.app(toCL(m.f, opts), toCL(m.x, opts));
  return C.abstractVar(m.n, toCL(m.b, opts), opts);
}
// Serializable AST for the Python oracle.
export const toJSON = (m) =>
  m.t === "v" ? m.n : m.t === "a" ? ["@", toJSON(m.f), toJSON(m.x)] : ["\\", m.n, toJSON(m.b)];

// ---- Church numerals and arithmetic ----------------------------------------------
export const church = (n) => lams(["s", "z"], Array.from({ length: n }).reduce((acc) => ap(v("s"), acc), v("z")));
export const plus = parse("\\m n s z. m s (n s z)");
export const times = parse("\\m n s. m (n s)");
export const power = parse("\\m n. n m");
export const succ = parse("\\n s z. s (n s z)");
export const pred = parse("\\n s z. n (\\g h. h (g s)) (\\u. z) (\\u. u)");

// ---- positions and constructive simulation of weak beta steps --------------------------
export function weakRedexPath(m, path = []) {
  if (m.t !== "a") return null;
  if (m.f.t === "l") return path;
  return weakRedexPath(m.f, [...path, 0]) ?? weakRedexPath(m.x, [...path, 1]);
}
export const lamAt = (m, path) => path.reduce((u, p) => (p === 0 ? u.f : u.x), m);

// For a lambda term with a weak redex, write out the contraction sequence in the circle
// calculus that takes T(M) to T(M') and check that it ends exactly at T(M').
export function simulateWeak(m, opts = {}) {
  const path = weakRedexPath(m);
  if (!path) return null;
  const red = lamAt(m, path);
  const d = C.deriveAbstraction(red.f.n, toCL(red.f.b, opts), toCL(red.x, opts));
  const before = toCL(m, opts);
  const after = toCL(weakStep(m), opts);
  const reached = C.replaceAt(before, path, d.final);
  return { before, after, ok: reached === after, steps: d.log.length, log: d.log };
}

// All lambda terms of a given size over a variable set (size counts nodes).
const lamMemo = new Map();
export function lambdaTerms(size, vars) {
  const key = size + ":" + vars.join(",");
  if (lamMemo.has(key)) return lamMemo.get(key);
  const out = [];
  if (size === 1) vars.forEach((x) => out.push(v(x)));
  else {
    for (const x of vars) for (const b of lambdaTerms(size - 1, vars)) out.push(lam(x, b));
    for (let k = 1; k < size - 1; k++) {
      const rs = lambdaTerms(size - 1 - k, vars);
      for (const f of lambdaTerms(k, vars)) for (const x of rs) out.push(ap(f, x));
    }
  }
  lamMemo.set(key, out);
  return out;
}
