// Construction traces: choosing a decomposition, and accounting for the correction.
// Every step carries an exactness claim that is checked, either by polynomial identity
// (for the general rewrite) or by evaluation (for the instance).
import { leaf, node, pureEval, pureSize } from "./dp.mjs";

// Polynomials in named variables with BigInt coefficients: Map from monomial key to coeff.
export class Poly {
  constructor(terms = new Map()) {
    this.t = terms;
  }
  static num(n) {
    return new Poly(n === 0n ? new Map() : new Map([["", BigInt(n)]]));
  }
  static v(name) {
    return new Poly(new Map([[name, 1n]]));
  }
  static mono(a, b) {
    const parts = [...(a ? a.split("*") : []), ...(b ? b.split("*") : [])].sort();
    return parts.join("*");
  }
  add(o, sign = 1n) {
    const r = new Map(this.t);
    for (const [k, c] of o.t) {
      const n = (r.get(k) ?? 0n) + sign * c;
      if (n === 0n) r.delete(k);
      else r.set(k, n);
    }
    return new Poly(r);
  }
  sub(o) {
    return this.add(o, -1n);
  }
  mul(o) {
    const r = new Poly();
    for (const [ka, ca] of this.t) for (const [kb, cb] of o.t) {
      const k = Poly.mono(ka, kb);
      const t = new Map(r.t);
      const n = (t.get(k) ?? 0n) + ca * cb;
      if (n === 0n) t.delete(k);
      else t.set(k, n);
      r.t = t;
    }
    return r;
  }
  isZero() {
    return this.t.size === 0;
  }
  eval(env) {
    let s = 0n;
    for (const [k, c] of this.t) {
      let m = c;
      if (k) for (const p of k.split("*")) m *= env[p];
      s += m;
    }
    return s;
  }
}

// Difference of squares: (c - d)(c + d) = c*c - d*d.
export function differenceOfSquaresIdentity() {
  const c = Poly.v("c");
  const d = Poly.v("d");
  return c.sub(d).mul(c.add(d)).sub(c.mul(c).sub(d.mul(d))).isZero();
}

// Four-fold identity valid for all integers, including odd sums:
//   4ab = (a + b)^2 - (a - b)^2.
export function polarizationIdentity() {
  const a = Poly.v("a");
  const b = Poly.v("b");
  const four = Poly.num(4n);
  const lhs = four.mul(a).mul(b);
  const rhs = a.add(b).mul(a.add(b)).sub(a.sub(b).mul(a.sub(b)));
  return lhs.sub(rhs).isZero();
}

// A construction trace for a * b when a + b is even. Returns the centre m, the offset d,
// the provisional estimate m*m, the exact correction -(d*d), and the pure-fragment terms
// for the original and the reorganised problem.
export function centreAndCorrect(a, b) {
  a = BigInt(a);
  b = BigInt(b);
  if ((a + b) % 2n !== 0n) throw new Error("centre is not an integer when a + b is odd");
  const m = (a + b) / 2n;
  const d = (b - a) / 2n;
  const original = node("*", leaf(a), leaf(b));
  const split = node("*", node("-", leaf(m), leaf(d)), node("+", leaf(m), leaf(d)));
  const squares = node("-", node("^", leaf(m), leaf(2)), node("^", leaf(d), leaf(2)));
  const estimate = m * m;
  const correction = -(d * d);
  const steps = [
    { kind: "problem", term: original, value: pureEval(original) },
    { kind: "choose centre and offset", note: `m = ${m}, d = ${d}`, exact: m - d === a && m + d === b },
    { kind: "rewrite", term: split, value: pureEval(split), exact: pureEval(split) === pureEval(original) },
    { kind: "rewrite by (c-d)(c+d) = c^2 - d^2", term: squares, value: pureEval(squares), exact: pureEval(squares) === pureEval(original) },
    { kind: "provisional estimate", value: estimate, error: pureEval(original) - estimate },
    { kind: "correction", value: correction, accounts: estimate + correction === pureEval(original) },
  ];
  return { m, d, estimate, correction, steps, pops: { original: pureSize(original), split: pureSize(split), squares: pureSize(squares) } };
}
