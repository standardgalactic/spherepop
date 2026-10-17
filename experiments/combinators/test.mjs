// Verification suite for the S/K circle calculus.
// Run from this directory:  node test.mjs
// Every group states what is checked and over what bound. Measured quantities are
// collected in `facts` and written to generated/facts.json for the essay.
import assert from "node:assert/strict";
import { spawnSync } from "node:child_process";
import { writeFileSync, mkdirSync } from "node:fs";
import { fileURLToPath } from "node:url";
import * as C from "./cl.mjs";
import * as L from "./lam.mjs";
import * as M from "./machines.mjs";
import * as D from "./dp.mjs";
import * as X from "./decompose.mjs";

const here = (p) => fileURLToPath(new URL(p, import.meta.url));
const facts = {};
const results = [];
function group(id, title, fn) {
  const t0 = performance.now();
  const note = fn() ?? "";
  const ms = Math.round(performance.now() - t0);
  results.push({ id, title, note, ms });
  console.log(`PASS ${id} ${title}${note ? " [" + note + "]" : ""} (${ms} ms)`);
}

const toWire = (t) => (t.kind === 0 ? t.name : [toWire(t.f), toWire(t.x)]);
const fromWire = (w) => (typeof w === "string" ? C.atom(w) : C.app(fromWire(w[0]), fromWire(w[1])));
function oracle(requests) {
  const r = spawnSync("python3", ["-I", here("oracle.py")], {
    input: JSON.stringify(requests),
    encoding: "utf8",
    maxBuffer: 1 << 30,
  });
  assert.equal(r.status, 0, r.stderr);
  return JSON.parse(r.stdout);
}

const x = C.atom("x");
const atoms3 = [C.S, C.K, x];
const termsUpTo = (n, atoms = atoms3) => {
  const out = [];
  for (let k = 1; k <= n; k++) for (const t of C.enumerate(k, atoms)) out.push(t);
  return out;
};
const catalan = (n) => Number(D.catalan(n));

// ------------------------------------------------------------------------------------
group("T1", "interning, printing and parsing", () => {
  const a = C.parse("S (K x) (S K K)");
  assert.equal(a, C.app(C.app(C.S, C.app(C.K, x)), C.app(C.app(C.S, C.K), C.K)));
  assert.equal(C.parse(C.show(a)), a);
  assert.equal(C.show(C.parse("S K K")), "I");
  assert.equal(C.parse("a b c"), C.A(C.atom("a"), C.atom("b"), C.atom("c")));
  for (const t of termsUpTo(5)) assert.equal(C.parse(C.show(t, { macroI: false })), t);
});

group("T2", "enumeration counts equal C(n-1) * a^n (interval DP and direct enumeration)", () => {
  for (let n = 1; n <= 7; n++) {
    const direct = [...C.enumerate(n, atoms3)].length;
    assert.equal(direct, catalan(n - 1) * 3 ** n);
    assert.equal(new Set([...C.enumerate(n, atoms3)]).size, direct, "enumeration has duplicates");
  }
  facts.enumeration = Object.fromEntries([1, 2, 3, 4, 5, 6, 7].map((n) => [n, catalan(n - 1) * 3 ** n]));
});

// ------------------------------------------------------------------------------------
group("T3", "size change of every redex contraction: K loses 2 + |y|, S gains |z|", () => {
  let redexes = 0;
  let kCount = 0;
  let sCount = 0;
  let growing = 0;
  for (const t of termsUpTo(6)) {
    for (const path of C.redexPaths(t)) {
      const u = C.at(t, path);
      const v = C.contract(u);
      redexes++;
      const dsize = v.size - u.size;
      const dleaves = v.leaves - u.leaves;
      if (u.rk === 1) {
        kCount++;
        assert.equal(dsize, -(2n + u.x.size));
        assert.equal(dleaves, -(1n + u.x.leaves));
      } else {
        sCount++;
        assert.equal(dsize, u.x.size);
        assert.equal(dleaves, u.x.leaves - 1n);
        if (dsize > 0n) growing++;
      }
    }
  }
  facts.sizeLaw = { redexes, kCount, sCount, growing };
  return `${redexes} redex occurrences`;
});

group("T4", "no fixed measure: some contraction strictly increases circle count", () => {
  const t = C.parse("S a b (K c d)");
  const u = C.contract(t);
  assert(u.size > t.size);
  facts.growExample = { before: C.show(t), after: C.show(u), sizeBefore: Number(t.size), sizeAfter: Number(u.size) };
});

// ------------------------------------------------------------------------------------
group("T5", "omega: W (I^n W) reaches W (I^(n+1) W) in exactly 2n+3 leftmost-outermost steps", () => {
  const W = C.parse("S I I");
  const Iw = (n) => {
    let u = W;
    for (let i = 0; i < n; i++) u = C.app(C.I, u);
    return u;
  };
  const rows = [];
  for (let n = 0; n <= 40; n++) {
    let u = C.app(W, Iw(n));
    assert.equal(u.size, BigInt(13 + 3 * n));
    for (let i = 0; i < 2 * n + 3; i++) {
      const nx = C.step(u, "lo");
      assert(nx !== null);
      u = nx;
    }
    assert.equal(u, C.app(W, Iw(n + 1)), `n = ${n}`);
    if (n < 6) rows.push({ n, steps: 2 * n + 3, size: Number(C.app(W, Iw(n)).size) });
  }
  const omega = C.app(W, W);
  assert.equal(omega, C.app(W, Iw(0)));
  const lo = C.normalize(omega, "lo", { maxSteps: 5000 });
  assert.equal(lo.status, "budget");
  const stepsTo = (n) => {
    let s = 0;
    for (let i = 0; i < n; i++) s += 2 * i + 3;
    return s;
  };
  facts.omega = { rows, stepsTo40: stepsTo(40) };
  // Every term on the leftmost-outermost path has a redex, so the path is infinite. By the
  // leftmost normalization theorem (cited) omega has no normal form.
});

group("T6", "omega under the other strategies: leftmost-innermost, rightmost-innermost and rightmost-outermost cycle", () => {
  const omega = C.parse("S I I (S I I)");
  const out = {};
  for (const s of ["lo", "li", "ri", "ro"]) {
    const r = C.normalize(omega, s, { maxSteps: 400, trace: true });
    out[s] = { status: r.status, steps: r.steps, period: r.period ?? null };
    if (r.status === "cycle") {
      assert.equal(r.period, 5);
      assert.equal(r.trace[r.trace.length - 1], r.trace[r.trace.length - 1 - r.period]);
    }
  }
  assert.equal(out.lo.status, "budget");
  assert.equal(out.li.status, "cycle");
  facts.omegaStrategies = out;
  const li = C.normalize(omega, "li", { trace: true });
  facts.omegaCycle = li.trace.slice(0, 6).map((u) => C.show(u));
});

group("T7", "strategy sensitivity: K a omega has the normal form a under outermost and diverges under innermost", () => {
  const t = C.parse("K a (S I I (S I I))");
  const a = C.atom("a");
  const r = {};
  for (const s of ["lo", "li", "ri", "ro"]) {
    const n = C.normalize(t, s, { maxSteps: 300 });
    r[s] = { status: n.status, steps: n.steps };
  }
  assert.equal(C.normalize(t, "lo").term, a);
  assert.equal(C.normalize(t, "lo").steps, 1);
  assert.equal(C.normalize(t, "ro").term, a);
  assert.equal(r.li.status, "cycle");
  assert.equal(r.ri.status, "cycle");
  facts.kOmega = r;
});

group("T8", "unbounded growth: (S (S I I) I)(S (S I I) I) never repeats and keeps growing", () => {
  const Mx = C.parse("S (S I I) I");
  let u = C.app(Mx, Mx);
  const seen = new Set();
  const sizes = [];
  for (let i = 0; i <= 600; i++) {
    assert(!seen.has(u.id), "repeated term");
    seen.add(u.id);
    if (i % 100 === 0) sizes.push({ step: i, size: Number(u.size), dag: C.dagSize(u) });
    u = C.step(u, "lo");
    assert(u !== null);
  }
  assert(sizes[sizes.length - 1].size > 10 * sizes[0].size);
  facts.growth = sizes;
});

// ------------------------------------------------------------------------------------
group("T9", "Church-Rosser by parallel reduction: for every term up to 5 leaves, every parallel reduct reduces in parallel to the Gross-Knuth development", () => {
  let terms = 0;
  let pairs = 0;
  let nontrivial = 0;
  for (const t of termsUpTo(5)) {
    terms++;
    const g = C.gk(t);
    const P = C.parReducts(t);
    assert(P.has(g.id), "development must be a parallel reduct");
    for (const a of P.values()) {
      pairs++;
      assert(C.parReducts(a).has(g.id), "triangle property fails");
    }
    if (P.size > 1) nontrivial++;
  }
  facts.triangle = { terms, pairs, nontrivial };
  return `${terms} terms, ${pairs} reducts`;
});

group("T10", "parallel reduction lies between one step and the reflexive transitive closure", () => {
  let checked = 0;
  for (const t of termsUpTo(4)) {
    const g = D.exploreGraph(t, { limit: 4000 });
    for (const a of C.parReducts(t).values()) {
      assert(g.nodes.has(a.id) || !g.complete, "parallel reduct not reachable by single steps");
      checked++;
    }
    for (const r of C.reducts(t)) assert(C.parReducts(t).has(r.id), "single step is not a parallel step");
  }
  return `${checked} parallel reducts`;
});

group("T11", "the one-step diamond fails; the smallest failures, and where it holds", () => {
  const joinable = (a, b) => {
    const A = new Set([a.id, ...C.reducts(a).map((u) => u.id)]);
    return [b, ...C.reducts(b)].some((u) => A.has(u.id));
  };
  let first = null;
  const failing = {};
  const checkedByN = {};
  for (let n = 1; n <= 6; n++) {
    failing[n] = 0;
    checkedByN[n] = 0;
    for (const t of C.enumerate(n, atoms3)) {
      const R = [...new Set(C.reducts(t))];
      checkedByN[n]++;
      let bad = false;
      for (let i = 0; i < R.length && !bad; i++) for (let j = i + 1; j < R.length; j++) if (!joinable(R[i], R[j])) {
        bad = true;
        if (!first) first = { term: C.show(t), left: C.show(R[i]), right: C.show(R[j]) };
        break;
      }
      if (bad) failing[n]++;
    }
  }
  assert(first, "expected a failure of the one-step diamond");
  // The same pair is joined by parallel reduction.
  const t = C.parse(first.term);
  const R = [...new Set(C.reducts(t))];
  const g = C.gk(t);
  for (const r of R) assert(C.parReducts(r).has(g.id));
  facts.diamond = { first, failing, checkedByN };
});

group("T12", "at most one normal form is reachable from any term (complete graphs, up to 5 leaves)", () => {
  let complete = 0;
  let incomplete = 0;
  let withNF = 0;
  let cyclic = 0;
  for (const t of termsUpTo(5)) {
    const g = D.exploreGraph(t, { limit: 4000 });
    if (!g.complete) {
      incomplete++;
      continue;
    }
    complete++;
    const a = D.analyseGraph(g);
    assert(a.normalForms.length <= 1, "two distinct normal forms");
    if (a.normalForms.length === 1) withNF++;
    if (!a.acyclic) cyclic++;
    assert(a.bellmanAgrees);
  }
  facts.uniqueNF = { complete, incomplete, withNF, cyclic };
  return `${complete} complete graphs, ${incomplete} incomplete`;
});

group("T13", "leftmost-outermost finds the normal form whenever another strategy does (bounded evidence for the cited theorem); includes every S/K term of 7 and 8 leaves", () => {
  const stats = { terms: 0, loNormal: 0, nonNormalAll: 0, innermostMisses: 0, rightOutermostMisses: 0, loOnlyViolations: 0, nfDisagreements: 0 };
  const opts = { maxSteps: 3000, maxSize: 50000n };
  const corpus = [...termsUpTo(5), ...C.enumerate(7, [C.S, C.K]), ...C.enumerate(8, [C.S, C.K])];
  const nonNormal = [];
  for (const t of corpus) {
    stats.terms++;
    const lo = C.normalize(t, "lo", opts);
    const others = ["li", "ri", "ro"].map((s) => C.normalize(t, s, opts));
    if (lo.status === "normal") {
      stats.loNormal++;
      for (const o of others) if (o.status === "normal" && o.term !== lo.term) stats.nfDisagreements++;
      if (others[0].status !== "normal" || others[1].status !== "normal") stats.innermostMisses++;
      if (others[2].status !== "normal") stats.rightOutermostMisses++;
    } else {
      if (others.some((o) => o.status === "normal")) stats.loOnlyViolations++;
      else {
        stats.nonNormalAll++;
        if (nonNormal.length < 4 && t.leaves <= 7n) nonNormal.push(C.show(t, { macroI: false }));
      }
    }
  }
  assert.equal(stats.loOnlyViolations, 0);
  assert.equal(stats.nfDisagreements, 0);
  facts.normalization = { ...stats, smallestNonNormal: nonNormal };
  return `${stats.terms} terms; ${stats.nonNormalAll} without a normal form within budget; innermost misses ${stats.innermostMisses}`;
});

group("T14", "Bellman recursions: shortest, longest and path counts agree with brute-force path enumeration", () => {
  let checked = 0;
  for (const t of termsUpTo(4)) {
    const g = D.exploreGraph(t, { limit: 3000 });
    if (!g.complete) continue;
    const a = D.analyseGraph(g);
    if (!a.acyclic) continue;
    let count = 0n;
    let longest = 0;
    let shortest = Infinity;
    const walk = (u, d) => {
      const rs = C.reducts(u);
      if (rs.length === 0) {
        count++;
        longest = Math.max(longest, d);
        shortest = Math.min(shortest, d);
        return;
      }
      for (const r of rs) walk(r, d + 1);
    };
    walk(t, 0);
    assert.equal(a.paths, count);
    assert.equal(a.longest, longest);
    assert.equal(a.shortest, shortest);
    checked++;
  }
  facts.bellman = { acyclicGraphs: checked };
  return `${checked} acyclic graphs`;
});

group("T15", "how often each strategy is optimal on strongly normalizing terms (all terms up to 6 leaves over S, K, x; 7 and 8 leaves over S, K)", () => {
  const stats = { terms: 0, lo: 0, li: 0, ri: 0, ro: 0, loWorse: 0, liWorse: 0, riWorse: 0, roWorse: 0 };
  let loExample = null;
  let liExample = null;
  const corpus15 = [...termsUpTo(6), ...C.enumerate(7, [C.S, C.K]), ...C.enumerate(8, [C.S, C.K])];
  for (const t of corpus15) {
    const g = D.exploreGraph(t, { limit: 3000 });
    if (!g.complete) continue;
    const a = D.analyseGraph(g);
    if (!a.acyclic) continue;
    stats.terms++;
    for (const s of ["lo", "li", "ri", "ro"]) {
      const r = C.normalize(t, s, { maxSteps: 10000 });
      assert.equal(r.status, "normal");
      assert(r.steps >= a.shortest && r.steps <= a.longest);
      if (r.steps === a.shortest) stats[s]++;
      else stats[s + "Worse"]++;
      if (s === "lo" && r.steps > a.shortest && !loExample) loExample = { term: C.show(t), lo: r.steps, best: a.shortest };
      if (s === "li" && r.steps > a.shortest && !liExample) liExample = { term: C.show(t), li: r.steps, best: a.shortest };
    }
  }
  facts.optimality = { ...stats, loExample, liExample };
});

// ------------------------------------------------------------------------------------
group("T16", "differential test against the independent Python reference: all strategies, all terms up to 6 leaves", () => {
  let seedR = 4242;
  const rndR = (n) => {
    seedR = (Math.imul(seedR, 1664525) + 1013904223) >>> 0;
    return (seedR >>> 8) % n;
  };
  const randomTerm = (leavesCount) => {
    if (leavesCount === 1) return atoms3[rndR(3)];
    const k = 1 + rndR(leavesCount - 1);
    return C.app(randomTerm(k), randomTerm(leavesCount - k));
  };
  const specials = ["S I I (S I I)", "K a (S I I (S I I))", "S (S I I) I (S (S I I) I)", "S S S (S S) S S"].map((p) => C.parse(p));
  const randoms = Array.from({ length: 20000 }, () => randomTerm(9 + rndR(8)));
  const terms = [...termsUpTo(6), ...C.enumerate(7, [C.S, C.K]), ...C.enumerate(8, [C.S, C.K]), ...specials, ...randoms];
  const reqs = [];
  const strategies = ["lo", "li", "ri", "ro"];
  for (const t of terms) for (const s of strategies) reqs.push({ op: "normalize", term: toWire(t), strategy: s, maxSteps: 120, maxSize: 2000 });
  const answers = oracle(reqs);
  let i = 0;
  const tally = { normal: 0, cycle: 0, budget: 0, size: 0 };
  for (const t of terms) for (const s of strategies) {
    const py = answers[i++];
    const js = C.normalize(t, s, { maxSteps: 120, maxSize: 2000n });
    assert.equal(js.status, py.status, `${C.show(t)} ${s}`);
    assert.equal(js.steps, py.steps, `${C.show(t)} ${s}`);
    assert.equal(js.term, fromWire(py.term), `${C.show(t)} ${s}`);
    if (js.status === "cycle") assert.equal(js.period, py.period);
    tally[js.status]++;
  }
  // Gross-Knuth development
  const gAnswers = oracle(terms.slice(0, 4000).map((t) => ({ op: "gk", term: toWire(t) })));
  terms.slice(0, 4000).forEach((t, k) => assert.equal(C.gk(t), fromWire(gAnswers[k])));
  facts.differential = { terms: terms.length, comparisons: reqs.length, tally, randomTerms: randoms.length };
  assert(tally.cycle > 0 && tally.budget + tally.size > 0, "corpus must include nonterminating behaviour");
  return `${reqs.length} comparisons`;
});

group("T17", "abstract machine agrees with the tree strategy; sharing reduces work", () => {
  let compared = 0;
  let sharedLess = 0;
  for (const t of termsUpTo(6)) {
    const tree = C.normalize(t, "lo", { maxSteps: 500, maxSize: 4000n });
    if (tree.status !== "normal") continue;
    const mach = C.nfLO(t, { maxSteps: 500 });
    assert.equal(mach.status, "normal");
    assert.equal(mach.term, tree.term);
    assert.equal(mach.steps, tree.steps);
    const shared = C.nfLO(t, { maxSteps: 500, memo: true });
    assert.equal(shared.term, tree.term);
    assert(shared.steps <= mach.steps);
    if (shared.steps < mach.steps) sharedLess++;
    compared++;
  }
  facts.machineAgreement = { compared, sharedLess };
  return `${compared} normalizing terms`;
});

// ------------------------------------------------------------------------------------
group("T18", "bracket abstraction theorem: ([x]M) N reduces to M[x:=N] by the designated reduction of exactly the predicted length", () => {
  const vars = ["x", "y", "f"].map((n) => C.atom(n));
  const Ns = [C.parse("n"), C.parse("S K"), C.parse("f y"), C.parse("S (K a) (K b)"), C.parse("K x")];
  let cases = 0;
  let totalSteps = 0;
  const reqs = [];
  const meta = [];
  for (let n = 1; n <= 5; n++) for (const Mt of C.enumerate(n, [...vars, C.S, C.K])) for (const name of ["x", "y"]) {
    meta.push({ Mt, name });
    reqs.push({ op: "abstract", name, term: toWire(Mt) });
  }
  const answers = oracle(reqs);
  meta.forEach(({ Mt, name }, i) => {
    assert.equal(C.abstractVar(name, Mt), fromWire(answers[i].term));
    assert.equal(C.abstractionSteps(name, Mt), answers[i].steps);
    for (const N of Ns) {
      const d = C.deriveAbstraction(name, Mt, N);
      assert.equal(d.final, C.subst(Mt, name, N));
      assert.equal(d.log.length, C.abstractionSteps(name, Mt));
      assert(!C.hasVar(C.abstractVar(name, Mt), name), "abstraction must remove the variable");
      cases++;
      totalSteps += d.log.length;
    }
  });
  facts.abstraction = { terms: meta.length, cases, totalSteps };
  return `${cases} instances`;
});

group("T19", "eta-optimised abstraction also satisfies the theorem (reachability), and is never larger", () => {
  const vars = ["x", "y"].map((n) => C.atom(n));
  let checked = 0;
  let smaller = 0;
  const N = C.parse("n");
  for (let n = 1; n <= 4; n++) for (const Mt of C.enumerate(n, [...vars, C.S, C.K])) {
    const plain = C.abstractVar("x", Mt);
    const eta = C.abstractVar("x", Mt, { eta: true });
    assert(eta.leaves <= plain.leaves);
    if (eta.leaves < plain.leaves) smaller++;
    const g = D.exploreGraph(C.app(eta, N), { limit: 3000 });
    if (!g.complete) continue;
    assert(g.nodes.has(C.subst(Mt, "x", N).id), "M[x:=N] not reachable");
    checked++;
  }
  facts.eta = { checked, smaller };
});

group("T20", "quadratic growth of plain abstraction, measured", () => {
  const rows = [];
  for (const n of [2, 4, 6, 8, 10, 12]) {
    const names = Array.from({ length: n }, (_, i) => "v" + i);
    let body = C.atom(names[0]);
    for (let i = 1; i < n; i++) body = C.app(body, C.atom(names[i]));
    let plain = body;
    let eta = body;
    for (let i = n - 1; i >= 0; i--) {
      plain = C.abstractVar(names[i], plain);
      eta = C.abstractVar(names[i], eta, { eta: true });
    }
    rows.push({ n, plain: Number(plain.leaves), eta: Number(eta.leaves) });
  }
  for (let n = 1; n <= 16; n++) {
    const names = Array.from({ length: n }, (_, i) => "v" + i);
    let body = C.atom(names[0]);
    for (let i = 1; i < n; i++) body = C.app(body, C.atom(names[i]));
    for (let i = n - 1; i >= 0; i--) body = C.abstractVar(names[i], body);
    assert.equal(3n * body.leaves, BigInt(n ** 3 + 6 * n ** 2 + 8 * n - 6), `n = ${n}`);
  }
  facts.abstractionGrowth = rows;
});

// ------------------------------------------------------------------------------------
group("T21", "translation: substitution lemma and alpha invariance over all lambda terms up to size 6", () => {
  let checked = 0;
  const Ns = [L.parse("y"), L.parse("\\y. x y"), L.parse("x"), L.parse("\\x. x")];
  for (let size = 1; size <= 6; size++) for (const m of L.lambdaTerms(size, ["x", "y"])) for (const N of Ns) {
    const lhs = L.toCL(L.subst(m, "x", N));
    const rhs = C.subst(L.toCL(m), "x", L.toCL(N));
    // capture-avoiding substitution may rename a bound variable; translation is alpha-invariant
    assert.equal(lhs, rhs, `${L.show(m)} [x := ${L.show(N)}]`);
    checked++;
  }
  facts.substLemma = { checked };
  return `${checked} substitutions`;
});

group("T22", "weak beta steps are simulated: T(M) reduces to T(M') by an explicit contraction sequence", () => {
  let steps = 0;
  let sim = 0;
  let totalContractions = 0;
  for (let size = 1; size <= 7; size++) for (const m of L.lambdaTerms(size, ["x", "y"])) {
    const r = L.simulateWeak(m);
    if (!r) continue;
    assert(r.ok, L.show(m));
    sim++;
    totalContractions += r.steps;
    steps++;
  }
  facts.weakSim = { simulated: sim, contractions: totalContractions };
  return `${sim} weak steps`;
});

group("T23", "reduction under a binder is not a circle reduction: T(\\x. (\\y. y) x) and T(\\x. x) are distinct normal forms, joined only after applying", () => {
  const m = L.parse("\\x. (\\y. y) x");
  const m2 = L.fullStep(m);
  assert(L.alphaEq(m2, L.parse("\\x. x")));
  const a = L.toCL(m);
  const b = L.toCL(m2);
  assert.notEqual(a, b);
  assert.equal(C.nfLO(a).term, a);
  assert.equal(C.nfLO(b).term, b);
  assert.equal(a, C.parse("S (K I) I"));
  assert.equal(b, C.I);
  const v = C.atom("v");
  assert.equal(C.nfLO(C.app(a, v)).term, v);
  assert.equal(C.nfLO(C.app(b, v)).term, v);
  // How common is the failure? Classify full steps under a binder over small terms:
  // joinable as they stand, joinable after applying k fresh variables (k = number of
  // leading lambdas), or not joinable by either route.
  const leading = (t) => (t.t === "l" ? 1 + leading(t.b) : 0);
  let underBinder = 0;
  let directlyJoinable = 0;
  let notJoinable = 0;
  let appliedJoinable = 0;
  let neverJoinable = 0;
  let neverJoinableBothNormalDistinct = 0;
  let undecided = 0;
  for (let size = 1; size <= 7; size++) for (const t of L.lambdaTerms(size, ["x", "y"])) {
    if (L.weakStep(t)) continue; // only terms whose reducible redexes lie under binders
    const n = L.fullStep(t);
    if (!n) continue;
    underBinder++;
    const A1 = L.toCL(t);
    const B1 = L.toCL(n);
    if (A1 === B1) { directlyJoinable++; continue; }
    const ga = D.exploreGraph(A1, { limit: 400 });
    const gb = D.exploreGraph(B1, { limit: 400 });
    if (!ga.complete || !gb.complete) { undecided++; continue; }
    if ([...ga.nodes.keys()].some((id) => gb.nodes.has(id))) { directlyJoinable++; continue; }
    notJoinable++;
    const k = leading(t);
    let ta = A1;
    let tb = B1;
    for (let i = 0; i < k; i++) {
      const fv = C.atom("w" + i + "_");
      ta = C.app(ta, fv);
      tb = C.app(tb, fv);
    }
    const ra = C.nfLO(ta, { maxSteps: 5000 });
    const rb = C.nfLO(tb, { maxSteps: 5000 });
    if (ra.status === "normal" && rb.status === "normal" && ra.term === rb.term) appliedJoinable++;
    else {
      neverJoinable++;
      if (ra.status === "normal" && rb.status === "normal" && ra.term !== rb.term) neverJoinableBothNormalDistinct++;
    }
  }
  assert.equal(directlyJoinable + notJoinable + undecided, underBinder);
  assert.equal(appliedJoinable + neverJoinable, notJoinable);
  facts.underBinder = { terms: underBinder, directlyJoinable, notJoinable, appliedJoinable, neverJoinable, neverJoinableBothNormalDistinct, undecided };
});

group("T24", "Church arithmetic through the translation agrees with integer arithmetic of the pure fragment", () => {
  const s = C.atom("s");
  const z = C.atom("z");
  const decode = (t) => {
    const r = C.nfLO(C.A(t, s, z), { maxSteps: 3e6 });
    assert.equal(r.status, "normal");
    let n = 0;
    let u = r.term;
    while (u.kind === 1) {
      assert.equal(u.f, s);
      n++;
      u = u.x;
    }
    assert.equal(u, z);
    return n;
  };
  const cl = (m) => L.toCL(m);
  let cases = 0;
  for (let a = 0; a <= 5; a++) for (let b = 0; b <= 5; b++) {
    const ca = cl(L.church(a));
    const cb = cl(L.church(b));
    assert.equal(decode(C.A(cl(L.plus), ca, cb)), a + b);
    assert.equal(decode(C.A(cl(L.times), ca, cb)), a * b);
    assert.equal(BigInt(decode(C.A(cl(L.power), ca, cb))), D.pureEval(D.node("^", D.leaf(a), D.leaf(b))));
    // the lambda calculus agrees, by normal-order reduction to a numeral
    const lp = L.run(L.apps(L.times, L.church(a), L.church(b), L.v("s"), L.v("z")), L.fullStep, 1e6);
    assert.equal(lp.status, "normal");
    cases += 3;
  }
  for (let a = 0; a <= 6; a++) {
    assert.equal(decode(C.app(cl(L.succ), cl(L.church(a)))), a + 1);
    assert.equal(decode(C.app(cl(L.pred), cl(L.church(a)))), Math.max(a - 1, 0));
    cases += 2;
  }
  facts.church = { cases };
  return `${cases} instances`;
});

// ------------------------------------------------------------------------------------
group("T25", "counter machines: reference interpreter, Python oracle, lambda reduction and circle reduction all agree", () => {
  const inputsFor = (k) => {
    const out = [];
    const rec = (i, cur) => {
      if (i === k) return out.push(cur.slice());
      for (const v of [0, 1, 2, 3]) rec(i + 1, [...cur, v]);
    };
    rec(0, []);
    return out;
  };
  const table = [];
  let runs = 0;
  for (const [name, p] of Object.entries(M.programs)) {
    if (!p.fn) continue;
    const arity = p.arity;
    const grid = inputsFor(arity);
    const reqs = grid.map((v) => ({ op: "machine", prog: p.prog, regs: Array.from({ length: p.k }, (_, i) => v[i] ?? 0), maxSteps: 100000 }));
    const py = oracle(reqs);
    // oracle translation of the compiled lambda term must equal the JS translation
    const compiled = M.compile(p.prog, p.k);
    const pyT = oracle([{ op: "tocl", ast: L.toJSON(compiled) }])[0];
    assert.equal(fromWire(pyT), L.toCL(compiled), `translation of ${name}`);
    grid.forEach((v, i) => {
      const regs = Array.from({ length: p.k }, (_, j) => v[j] ?? 0);
      const ref = M.runMachine(p.prog, regs);
      assert(ref.halted);
      assert.equal(py[i].halted, true);
      assert.equal(py[i].steps, ref.steps);
      assert.deepEqual(py[i].regs, ref.regs);
      assert.equal(ref.regs[0], p.fn(...v.slice(0, arity)), `${name}(${v})`);
      const lam = M.decodeLam(M.initial(p.prog, p.k, regs));
      assert.equal(lam.value, ref.regs[0]);
      const T = L.toCL(M.initial(p.prog, p.k, regs));
      const cl = M.decodeCL(T, { maxSteps: 5e7 });
      assert.equal(cl.status, "normal");
      assert.equal(cl.value, ref.regs[0]);
      runs++;
      if (i === grid.length - 1 || (v.join() === (arity === 1 ? "3" : "3,2"))) table.push({ name, input: v.join(","), machineSteps: ref.steps, lambdaSteps: lam.steps, circleSteps: cl.steps, output: cl.value });
    });
    facts["size_" + name] = { states: p.prog.length, registers: p.k, circles: Number(L.toCL(compiled).size), dag: C.dagSize(L.toCL(compiled)), circlesEta: Number(L.toCL(compiled, { eta: true }).size) };
  }
  facts.machines = { runs, table };
  return `${runs} runs`;
});

group("T26", "a non-halting machine: the reference does not halt, and neither lambda nor circle evaluation produces a normal form within budget", () => {
  const p = M.programs.loop;
  const ref = M.runMachine(p.prog, [0], 100000);
  assert.equal(ref.halted, false);
  const lam = M.decodeLam(M.initial(p.prog, p.k, [0]), { maxSteps: 20000 });
  assert.equal(lam.status, "budget");
  const T = L.toCL(M.initial(p.prog, p.k, [0]));
  const cl = M.decodeCL(T, { maxSteps: 200000 });
  assert.equal(cl.status, "budget");
  // The leftmost-outermost term always has a head redex while it runs: check a long prefix.
  let u = T;
  for (let i = 0; i < 20000; i++) {
    const n = C.step(u, "lo");
    assert(n !== null);
    u = n;
  }
  facts.divergence = { prefix: 20000 };
});

group("T27", "undecidability witness: a program whose halting depends on its input", () => {
  // r0 counts down by 2 and halts exactly on even inputs; odd inputs loop forever.
  const prog = [
    { op: "dec", r: 0, nz: 1, z: 3 },
    { op: "dec", r: 0, nz: 0, z: 2 },
    { op: "inc", r: 0, next: 2 },
    { op: "halt" },
  ];
  for (const n of [0, 2, 4, 6]) {
    assert.equal(M.runMachine(prog, [n], 1e5).halted, true);
    assert.equal(M.decodeCL(L.toCL(M.initial(prog, 1, [n])), { maxSteps: 1e7 }).status, "normal");
  }
  for (const n of [1, 3, 5]) {
    assert.equal(M.runMachine(prog, [n], 1e5).halted, false);
    assert.equal(M.decodeCL(L.toCL(M.initial(prog, 1, [n])), { maxSteps: 200000 }).status, "budget");
  }
});

// ------------------------------------------------------------------------------------
group("T28", "sharing is memoization: graph reduction (head normal forms memoized by node) does strictly less work than tree reduction on the doubling chain, and the ratio grows", () => {
  const rows = [];
  const dbl = L.parse("\\x. (\\m n s z. m s (n s z)) x x");
  for (const n of [1, 2, 3, 4, 6, 8, 10, 12]) {
    let m = L.church(1);
    for (let i = 0; i < n; i++) m = L.ap(dbl, m);
    const t = C.A(L.toCL(m), C.atom("s"), C.atom("z"));
    const tree = C.nfLO(t, { maxSteps: 5e7 });
    const shared = C.nfShared(t, { maxSteps: 5e7 });
    assert.equal(tree.status, "normal");
    assert.equal(shared.status, "normal");
    assert.equal(shared.term, tree.term);
    assert.equal(tree.term.leaves - 1n >= BigInt(2 ** n), true);
    rows.push({ n, value: 2 ** n, treeSteps: tree.steps, sharedSteps: shared.steps, nfCircles: Number(tree.term.size), nfDag: C.dagSize(tree.term) });
  }
  for (let i = 1; i < rows.length; i++) assert(rows[i].sharedSteps <= rows[i].treeSteps);
  for (let i = 3; i < rows.length; i++) assert(rows[i].treeSteps / rows[i].sharedSteps > rows[i - 1].treeSteps / rows[i - 1].sharedSteps);
  facts.sharing = rows;
});

group("T29", "the pure fragment: pop orderings by memoized DP equal the hook length formula; states equal order ideals", () => {
  let seed = 20261017;
  const rnd = (n) => {
    seed = (Math.imul(seed, 1664525) + 1013904223) >>> 0;
    return seed % n;
  };
  const gen = (c) => {
    if (c === 0) return D.leaf(rnd(5));
    const k = rnd(c);
    return D.node(["+", "-", "*"][rnd(3)], gen(k), gen(c - 1 - k));
  };
  let trees = 0;
  const rows = [];
  for (let c = 1; c <= 9; c++) for (let rep = 0; rep < 25; rep++) {
    const t = gen(c);
    // distinct leaf values keep distinct positions distinct as states; use them
    const ct = D.pureCount(t);
    assert.equal(D.hookOrderings(t), ct.orderings, D.pureKey(t));
    assert(ct.states <= Number(D.idealCount(t)));
    // every pop removes exactly one circle and independent pops commute
    for (const p of D.pureReady(t)) assert.equal(D.pureSize(D.purePop(t, p)), D.pureSize(t) - 1);
    const rs = D.pureReady(t);
    for (let i = 0; i < rs.length; i++) for (let j = i + 1; j < rs.length; j++) assert.equal(D.pureKey(D.purePop(D.purePop(t, rs[i]), rs[j])), D.pureKey(D.purePop(D.purePop(t, rs[j]), rs[i])));
    trees++;
    if (rep === 0) rows.push({ circles: c, orderings: String(ct.orderings), ideals: String(D.idealCount(t)) });
  }
  // balanced trees: seven circles
  const bal = D.node("+", D.node("+", D.node("+", D.leaf(1), D.leaf(1)), D.node("+", D.leaf(1), D.leaf(1))), D.node("+", D.node("+", D.leaf(1), D.leaf(1)), D.node("+", D.leaf(1), D.leaf(1))));
  assert.equal(D.hookOrderings(bal), 80n);
  assert.equal(D.idealCount(bal), 26n);
  const two = D.node("+", D.node("*", D.leaf(2), D.leaf(3)), D.node("*", D.leaf(4), D.leaf(5)));
  assert.equal(D.pureCount(two).orderings, 2n);
  assert.equal(D.pureCount(two).states, 5);
  facts.pure = { trees, balancedOrderings: "80", balancedIdeals: "26" };
  return `${trees} random trees`;
});

group("T30", "interval DP over parenthesizations equals brute-force enumeration and sums to Catalan numbers", () => {
  const cases = [
    [2, "+", 3, "*", 4],
    [2, "^", 2, "^", 2, "^", 2],
    [8, "-", 3, "-", 2],
    [1, "+", 2, "*", 3, "-", 4, "*", 5],
    [2, "*", 3, "+", 4, "*", 5, "-", 6],
    [3, "^", 2, "*", 2, "-", 1],
  ];
  let seed = 7;
  const rnd = (n) => {
    seed = (Math.imul(seed, 1103515245) + 12345) >>> 0;
    return (seed >>> 8) % n;
  };
  for (let i = 0; i < 40; i++) {
    const m = 2 + rnd(5);
    const tokens = [rnd(5)];
    for (let k = 0; k < m; k++) tokens.push(["+", "-", "*", "^"][rnd(4)], rnd(4));
    cases.push(tokens);
  }
  for (const tk of cases) {
    const dp = D.parenthesizations(tk);
    const bf = D.bruteParenthesizations(tk);
    assert.equal(dp.total, D.catalan((tk.length - 1) / 2));
    assert.equal(bf.total, dp.total);
    assert.equal(dp.excluded, bf.excluded);
    assert.equal(dp.values.size, bf.values.size);
    for (const [v, c] of bf.values) assert.equal(dp.values.get(v), c);
  }
  const a = D.parenthesizations([2, "+", 3, "*", 4]).values;
  assert.equal(a.get(14n), 1n);
  assert.equal(a.get(20n), 1n);
  const tower = D.parenthesizations([2, "^", 2, "^", 2, "^", 2]);
  assert.equal(tower.values.get(256n), 3n);
  assert.equal(tower.values.get(65536n), 2n);
  facts.grouping = {
    expressions: cases.length,
    sumProduct: [...a].map(([v, c]) => [String(v), String(c)]),
    tower: [...tower.values].map(([v, c]) => [String(v), String(c)]),
    minus: [...D.parenthesizations([8, "-", 3, "-", 2]).values].map(([v, c]) => [String(v), String(c)]),
    catalan: [1, 2, 3, 4, 5, 6, 7, 8].map((n) => String(D.catalan(n))),
  };
  return `${cases.length} expressions`;
});

group("T31", "interval DP for the number of circle structures of a flat word of S, K and variables", () => {
  for (let n = 1; n <= 7; n++) assert.equal(D.catalan(n - 1) * 3n ** BigInt(n), BigInt([...C.enumerate(n, atoms3)].length));
});

group("T32", "decomposition: difference of squares, polarization, and the centre-and-correct trace", () => {
  assert(X.differenceOfSquaresIdentity());
  assert(X.polarizationIdentity());
  let pairs = 0;
  for (let a = 1; a <= 99; a++) for (let b = 1; b <= 99; b++) {
    const A = BigInt(a);
    const B = BigInt(b);
    assert.equal(4n * A * B, (A + B) ** 2n - (A - B) ** 2n);
    if ((a + b) % 2 === 0) {
      const t = X.centreAndCorrect(a, b);
      assert.equal(t.estimate + t.correction, A * B);
      assert.equal(-t.correction, t.d * t.d);
      assert(t.steps.every((s) => s.exact !== false && s.accounts !== false));
      pairs++;
    }
  }
  const t = X.centreAndCorrect(48, 52);
  assert.equal(t.m, 50n);
  assert.equal(t.d, 2n);
  assert.equal(t.estimate, 2500n);
  assert.equal(t.correction, -4n);
  assert.equal(t.steps[0].value, 2496n);
  const u = X.centreAndCorrect(997, 1003);
  assert.equal(u.estimate + u.correction, 999991n);
  facts.decompose = { pairs, example: { m: 50, d: 2, estimate: 2500, correction: -4, value: 2496, pops: t.pops } };
  return `${pairs} same-parity pairs`;
});

// ------------------------------------------------------------------------------------
// Optional: agree with the existing circles laboratory when it is present. Leaves carry
// distinct values so that states counted by term identity coincide with order ideals.
let optional = "skipped";
try {
  const E = await import("../circles/engine.mjs");
  const OPNAME = { "+": "add", "-": "sub", "*": "mul" };
  const primes = [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37];
  let seed = 99;
  const rnd = (n) => {
    seed = (Math.imul(seed, 1664525) + 1013904223) >>> 0;
    return seed % n;
  };
  let used = 0;
  const genPair = (c) => {
    if (c === 0) {
      const p = primes[used++];
      return [D.leaf(p), E.num(p)];
    }
    const k = rnd(c);
    const op = ["+", "-", "*"][rnd(3)];
    const [l1, l2] = genPair(k);
    const [r1, r2] = genPair(c - 1 - k);
    return [D.node(op, l1, r1), E.circle(OPNAME[op], l2, r2)];
  };
  let compared = 0;
  for (let c = 1; c <= 8; c++) for (let rep = 0; rep < 10; rep++) {
    used = 0;
    const [mine, theirs] = genPair(c);
    const g = E.explore(theirs, 100000);
    const count = D.pureCount(mine);
    assert(g.complete);
    assert.equal(g.states, count.states);
    assert.equal(g.states, Number(D.idealCount(mine)));
    assert.equal(g.normalForms.length, 1);
    assert.equal(BigInt(g.normalForms[0].value), D.pureEval(mine));
    const n = E.normalize(theirs);
    assert.equal(n.pops, D.pureSize(mine));
    assert.equal(E.size(theirs), D.pureSize(mine));
    compared++;
  }
  optional = `agreed on ${compared} random trees`;
} catch (e) {
  if (e.code !== "ERR_MODULE_NOT_FOUND") throw e;
}
facts.circlesLab = optional;
console.log(`T33 optional cross-check with ../circles/engine.mjs: ${optional}`);

mkdirSync(here("generated"), { recursive: true });
writeFileSync(here("generated/facts.json"), JSON.stringify(facts, (_, v) => (typeof v === "bigint" ? String(v) : v), 2));
writeFileSync(here("generated/verification.json"), JSON.stringify(results, null, 2));
console.log(`\n${results.length} verification groups passed.`);
