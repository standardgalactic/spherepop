// Dynamic-programming views of reduction.
//  1. The reduction graph of an S/K term, with Bellman recursions for shortest and
//     longest reduction and for the number of reduction paths.
//  2. The pure fragment: counting pop orderings by DP over states and by the hook
//     length formula for forests.
//  3. Interval DP over the parenthesizations of a flat expression.
import * as C from "./cl.mjs";

// ---------------------------------------------------------------------------------
// 1. Reduction graphs
// ---------------------------------------------------------------------------------
export function exploreGraph(t, { limit = 20000 } = {}) {
  const nodes = new Map([[t.id, t]]);
  const adj = new Map();
  const queue = [t];
  let complete = true;
  for (let i = 0; i < queue.length; i++) {
    const u = queue[i];
    const out = [];
    for (const r of C.reducts(u)) {
      if (!nodes.has(r.id)) {
        if (nodes.size >= limit) {
          complete = false;
          continue;
        }
        nodes.set(r.id, r);
        queue.push(r);
      }
      out.push(r.id);
    }
    adj.set(u.id, out);
  }
  return { root: t, nodes, adj, complete };
}

// Analyse a complete graph. Values per node:
//   shortest: fewest contractions to some normal form (Bellman recursion)
//   longest:  most contractions on a maximal path (Infinity if a cycle is reachable)
//   paths:    number of maximal reduction paths counted by redex position (BigInt),
//             or Infinity when a cycle is reachable
export function analyseGraph(g) {
  if (!g.complete) throw new Error("graph is incomplete");
  const { nodes, adj, root } = g;
  const normalForms = [...nodes.values()].filter((u) => !u.has);

  // Iterative DFS computing longest and paths in post-order, detecting cycles.
  const longest = new Map();
  const paths = new Map();
  const state = new Map(); // 1 = on stack, 2 = done
  let acyclic = true;
  const stack = [[root.id, 0]];
  state.set(root.id, 1);
  while (stack.length) {
    const top = stack[stack.length - 1];
    const [id, idx] = top;
    const out = adj.get(id);
    if (idx < out.length) {
      top[1]++;
      const w = out[idx];
      const sw = state.get(w);
      if (sw === undefined) {
        state.set(w, 1);
        stack.push([w, 0]);
      } else if (sw === 1) acyclic = false;
    } else {
      stack.pop();
      state.set(id, 2);
      if (out.length === 0) {
        longest.set(id, 0);
        paths.set(id, 1n);
      } else {
        let L = 0;
        let P = 0n;
        let inf = false;
        for (const w of out) {
          const lw = longest.get(w);
          if (lw === undefined) {
            inf = true; // successor on the stack: a cycle
            continue;
          }
          if (lw === Infinity) inf = true;
          else L = Math.max(L, 1 + lw);
          const pw = paths.get(w);
          if (pw === Infinity) inf = true;
          else P += pw;
        }
        longest.set(id, inf ? Infinity : L);
        paths.set(id, inf ? Infinity : P);
      }
    }
  }

  // Bellman value iteration: V(u) = 0 at normal forms, else 1 + min over successors.
  const V = new Map();
  for (const id of nodes.keys()) V.set(id, nodes.get(id).has ? Infinity : 0);
  for (let changed = true; changed; ) {
    changed = false;
    for (const [id, out] of adj) {
      if (out.length === 0) continue;
      let best = Infinity;
      for (const w of out) best = Math.min(best, 1 + V.get(w));
      if (best < V.get(id)) {
        V.set(id, best);
        changed = true;
      }
    }
  }
  // Independent computation: multi-source BFS backwards from the normal forms.
  const rev = new Map();
  for (const [id, out] of adj) for (const w of out) (rev.get(w) ?? rev.set(w, []).get(w)).push(id);
  const D = new Map();
  let frontier = normalForms.map((u) => u.id);
  frontier.forEach((id) => D.set(id, 0));
  for (let d = 0; frontier.length; d++) {
    const next = [];
    for (const w of frontier) for (const id of rev.get(w) ?? []) if (!D.has(id)) {
      D.set(id, d + 1);
      next.push(id);
    }
    frontier = next;
  }
  let bellmanAgrees = true;
  for (const id of nodes.keys()) if ((D.get(id) ?? Infinity) !== V.get(id)) bellmanAgrees = false;

  let edges = 0;
  for (const out of adj.values()) edges += out.length;
  const normalForm = normalForms.length === 1 ? normalForms[0] : null;
  return {
    states: nodes.size,
    edges,
    normalForms,
    normalForm,
    acyclic,
    shortest: V.get(root.id),
    longest: longest.get(root.id),
    paths: paths.get(root.id),
    bellmanAgrees,
    values: V,
  };
}

// ---------------------------------------------------------------------------------
// 2. The pure fragment: binary arithmetic circles over integer leaves
// ---------------------------------------------------------------------------------
export const leaf = (n) => ({ leaf: BigInt(n) });
export const node = (op, l, r) => ({ op, l, r });
const OPS = {
  "+": (a, b) => a + b,
  "-": (a, b) => a - b,
  "*": (a, b) => a * b,
  "^": (a, b) => {
    if (b < 0n || b > 4096n) throw new Error("exponent outside budget");
    return a ** b;
  },
};
export const pureKey = (t) => (t.leaf !== undefined ? String(t.leaf) : `(${t.op} ${pureKey(t.l)} ${pureKey(t.r)})`);
export const pureSize = (t) => (t.leaf !== undefined ? 0 : 1 + pureSize(t.l) + pureSize(t.r));
export function pureReady(t, path = [], out = []) {
  if (t.leaf !== undefined) return out;
  if (t.l.leaf !== undefined && t.r.leaf !== undefined) out.push(path);
  else {
    pureReady(t.l, [...path, 0], out);
    pureReady(t.r, [...path, 1], out);
  }
  return out;
}
export function purePop(t, path, i = 0) {
  if (i === path.length) return leaf(OPS[t.op](t.l.leaf, t.r.leaf));
  return path[i] === 0 ? node(t.op, purePop(t.l, path, i + 1), t.r) : node(t.op, t.l, purePop(t.r, path, i + 1));
}
export function pureEval(t) {
  while (pureSize(t) > 0) t = purePop(t, pureReady(t)[0]);
  return t.leaf;
}

// Number of pop orderings and number of reachable states, by memoized recursion.
export function pureCount(t) {
  const memo = new Map();
  const go = (u) => {
    const k = pureKey(u);
    let r = memo.get(k);
    if (r === undefined) {
      const rs = pureReady(u);
      r = rs.length === 0 ? 1n : rs.reduce((s, p) => s + go(purePop(u, p)), 0n);
      memo.set(k, r);
    }
    return r;
  };
  return { orderings: go(t), states: memo.size };
}

// Hook length formula for forests (Knuth, TAOCP 3, 5.1.4): the number of linear
// extensions of the poset in which every circle follows the circles inside it is
// n! / product of subtree sizes.
export function hookOrderings(t) {
  const n = pureSize(t);
  let num = 1n;
  for (let i = 2n; i <= BigInt(n); i++) num *= i;
  let den = 1n;
  const walk = (u) => {
    if (u.leaf !== undefined) return 0;
    const s = 1 + walk(u.l) + walk(u.r);
    den *= BigInt(s);
    return s;
  };
  walk(t);
  return num / den;
}
// Number of order ideals of the same poset: I(circle) = 1 + product of I(children).
export function idealCount(t) {
  if (t.leaf !== undefined) return 1n;
  return 1n + idealCount(t.l) * idealCount(t.r);
}

// ---------------------------------------------------------------------------------
// 3. Interval DP over parenthesizations
// ---------------------------------------------------------------------------------
// tokens = [n0, op1, n1, ..., opm, nm]. Returns a Map from value to the number of
// parenthesizations producing it. Parenthesizations whose evaluation leaves the
// integer domain (negative or huge exponents) are counted in `excluded`.
export function parenthesizations(tokens) {
  const nums = tokens.filter((_, i) => i % 2 === 0).map(BigInt);
  const ops = tokens.filter((_, i) => i % 2 === 1);
  const m = nums.length;
  const table = Array.from({ length: m }, () => Array(m).fill(null));
  const bad = Array.from({ length: m }, () => Array(m).fill(0n));
  const total = Array.from({ length: m }, () => Array(m).fill(0n));
  for (let i = 0; i < m; i++) {
    table[i][i] = new Map([[nums[i], 1n]]);
    total[i][i] = 1n;
  }
  for (let len = 2; len <= m; len++)
    for (let i = 0; i + len - 1 < m; i++) {
      const j = i + len - 1;
      const cell = new Map();
      let excluded = 0n;
      let all = 0n;
      for (let k = i; k < j; k++) {
        const op = ops[k];
        const left = table[i][k];
        const right = table[k + 1][j];
        all += total[i][k] * total[k + 1][j];
        excluded += bad[i][k] * total[k + 1][j] + total[i][k] * bad[k + 1][j];
        for (const [a, ca] of left) for (const [b, cb] of right) {
          let val;
          try {
            val = OPS[op](a, b);
          } catch {
            excluded += ca * cb;
            continue;
          }
          cell.set(val, (cell.get(val) ?? 0n) + ca * cb);
        }
      }
      table[i][j] = cell;
      bad[i][j] = excluded;
      total[i][j] = all;
    }
  return { values: table[0][m - 1], total: total[0][m - 1], excluded: bad[0][m - 1] };
}

export function catalan(n) {
  let c = 1n;
  for (let i = 0n; i < BigInt(n); i++) c = (c * 2n * (2n * i + 1n)) / (i + 2n);
  return c;
}

// Brute force over explicit trees, evaluated by the pure engine, for cross-checking.
export function bruteParenthesizations(tokens) {
  const nums = tokens.filter((_, i) => i % 2 === 0).map(BigInt);
  const ops = tokens.filter((_, i) => i % 2 === 1);
  const build = (i, j) => {
    if (i === j) return [leaf(nums[i])];
    const out = [];
    for (let k = i; k < j; k++) for (const l of build(i, k)) for (const r of build(k + 1, j)) out.push(node(ops[k], l, r));
    return out;
  };
  const values = new Map();
  let excluded = 0n;
  let total = 0n;
  for (const t of build(0, nums.length - 1)) {
    total++;
    try {
      const v = pureEval(t);
      values.set(v, (values.get(v) ?? 0n) + 1n);
    } catch {
      excluded++;
    }
  }
  return { values, total, excluded };
}
