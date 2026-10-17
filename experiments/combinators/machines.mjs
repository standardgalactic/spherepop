// Counter machines, a reference interpreter, and a compiler into lambda terms
// (Scott numerals, a call-by-name fixed point, a dispatch chain), which the translation
// of lam.mjs then maps to S/K circles.
import * as L from "./lam.mjs";
import * as C from "./cl.mjs";

// Instructions: {op:"inc", r, next}, {op:"dec", r, nz, z}, {op:"halt"}.
// "dec" decrements register r and goes to nz when it was positive, else goes to z.
export function runMachine(prog, regs, maxSteps = 1e6) {
  regs = regs.slice();
  let pc = 0;
  let steps = 0;
  const trace = [];
  while (steps < maxSteps) {
    const ins = prog[pc];
    if (!ins || ins.op === "halt") return { halted: true, steps, regs, trace };
    if (ins.op === "inc") {
      regs[ins.r]++;
      pc = ins.next;
    } else if (regs[ins.r] > 0) {
      regs[ins.r]--;
      pc = ins.nz;
    } else pc = ins.z;
    steps++;
  }
  return { halted: false, steps, regs, trace };
}

const zeroL = L.parse("\\z t. z");
export const scott = (n) => (n === 0 ? zeroL : L.lams(["z", "t"], L.ap(L.v("t"), scott(n - 1))));
const succL = L.parse("\\n z t. t n");
const Y = L.parse("\\f. (\\x. f (x x)) (\\x. f (x x))");

export function compile(prog, k) {
  const m = prog.length;
  const rs = Array.from({ length: k }, (_, i) => "r" + i);
  const call = (state, regTerms) => L.apps(L.v("f"), scott(state), ...regTerms);
  const regsOf = () => rs.map((r) => L.v(r));
  const body = (ins) => {
    if (!ins || ins.op === "halt") return L.v("r0");
    if (ins.op === "inc") {
      const t = regsOf();
      t[ins.r] = L.ap(succL, t[ins.r]);
      return call(ins.next, t);
    }
    const zeroBranch = call(ins.z, regsOf());
    const t = regsOf();
    t[ins.r] = L.v("q");
    const succBranch = L.lam("q", call(ins.nz, t));
    return L.apps(L.v(rs[ins.r]), zeroBranch, succBranch);
  };
  // Dispatch on the state numeral: s B0 (\p0. p0 B1 (\p1. p1 B2 (...))). Beyond the last
  // state the machine halts.
  const dispatch = (i, sv) => {
    const next = i === m - 1 ? L.lam("pe", body(null)) : L.lam("p" + i, dispatch(i + 1, "p" + i));
    return L.apps(L.v(sv), body(prog[i]), next);
  };
  const F = L.lams(["f", "s", ...rs], dispatch(0, "s"));
  return L.ap(Y, F);
}

export function initial(prog, k, inputs) {
  const regs = Array.from({ length: k }, (_, i) => inputs[i] ?? 0);
  return L.apps(compile(prog, k), scott(0), ...regs.map(scott));
}

// ---- decoding Scott numerals ---------------------------------------------------------
export function decodeCL(term, { maxSteps = 5e6, limit = 10000 } = {}) {
  const Z = C.atom("zz");
  const Sx = C.atom("ss");
  let n = 0;
  let cur = term;
  let total = 0;
  for (;;) {
    const r = C.nfLO(C.A(cur, Z, Sx), { maxSteps });
    total += r.steps;
    if (r.status !== "normal") return { status: r.status, value: null, steps: total };
    if (r.term === Z) return { status: "normal", value: n, steps: total };
    if (r.term.kind === 1 && r.term.f === Sx) {
      n++;
      cur = r.term.x;
      if (n > limit) return { status: "limit", value: null, steps: total };
    } else return { status: "malformed", value: null, steps: total, got: C.show(r.term) };
  }
}
export function decodeLam(m, { maxSteps = 1e6, limit = 10000 } = {}) {
  let n = 0;
  let cur = m;
  let total = 0;
  for (;;) {
    const r = L.run(L.apps(cur, L.v("zz"), L.v("ss")), L.weakStep, maxSteps);
    total += r.steps;
    if (r.status !== "normal") return { status: r.status, value: null, steps: total };
    if (r.term.t === "v" && r.term.n === "zz") return { status: "normal", value: n, steps: total };
    if (r.term.t === "a" && r.term.f.t === "v" && r.term.f.n === "ss") {
      n++;
      cur = r.term.x;
      if (n > limit) return { status: "limit", value: null, steps: total };
    } else return { status: "malformed", value: null, steps: total };
  }
}

// ---- programs -----------------------------------------------------------------------
// Each program lists its register count, a description, and the function it computes
// on register 0 given the inputs.
export const programs = {
  add: {
    arity: 2,
    k: 2,
    note: "r0 := r0 + r1",
    prog: [
      { op: "dec", r: 1, nz: 1, z: 2 },
      { op: "inc", r: 0, next: 0 },
      { op: "halt" },
    ],
    fn: (a, b) => a + b,
  },
  double: {
    arity: 1,
    k: 2,
    note: "r0 := 2 r0, using r1 as scratch",
    prog: [
      { op: "dec", r: 0, nz: 1, z: 3 },
      { op: "inc", r: 1, next: 2 },
      { op: "inc", r: 1, next: 0 },
      { op: "dec", r: 1, nz: 4, z: 5 },
      { op: "inc", r: 0, next: 3 },
      { op: "halt" },
    ],
    fn: (a) => 2 * a,
  },
  monus: {
    arity: 2,
    k: 2,
    note: "r0 := max(r0 - r1, 0)",
    prog: [
      { op: "dec", r: 1, nz: 1, z: 3 },
      { op: "dec", r: 0, nz: 0, z: 2 },
      { op: "halt" },
      { op: "halt" },
    ],
    fn: (a, b) => Math.max(a - b, 0),
  },
  mult: {
    arity: 2,
    k: 4,
    note: "r0 := r0 * r1 using r2 and r3 as scratch",
    prog: [],
    fn: (a, b) => a * b,
  },
  parity: {
    arity: 1,
    k: 1,
    note: "r0 := 1 if r0 is odd else 0",
    prog: [
      { op: "dec", r: 0, nz: 1, z: 3 },
      { op: "dec", r: 0, nz: 0, z: 2 },
      { op: "inc", r: 0, next: 3 },
      { op: "halt" },
    ],
    fn: (a) => a % 2,
  },
  loop: {
    arity: 1,
    k: 1,
    note: "increments r0 forever",
    prog: [{ op: "inc", r: 0, next: 0 }],
    fn: null,
  },
};

// Multiplication: r2 accumulates r0*r1; r3 restores r1.
programs.mult.prog = [
  /*0*/ { op: "dec", r: 0, nz: 1, z: 7 }, // for each unit of r0
  /*1*/ { op: "dec", r: 1, nz: 2, z: 4 }, // add r1 into r2, saving it in r3
  /*2*/ { op: "inc", r: 2, next: 3 },
  /*3*/ { op: "inc", r: 3, next: 1 },
  /*4*/ { op: "dec", r: 3, nz: 5, z: 0 }, // restore r1 from r3
  /*5*/ { op: "inc", r: 1, next: 4 },
  /*6*/ { op: "halt" },
  /*7*/ { op: "dec", r: 2, nz: 8, z: 9 }, // move r2 into r0
  /*8*/ { op: "inc", r: 0, next: 7 },
  /*9*/ { op: "halt" },
];
