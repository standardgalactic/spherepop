// The pure, many-sorted fragment of An Introduction to SpherePOP, chapters 1–7.
// BigInt keeps numerical values integral, including the book's power towers.
export const num = (value) => ({ kind: "num", value: String(BigInt(value)) });
export const sym = (name) => ({ kind: "sym", name });
export const color = (name) => ({ kind: "color", name });
export const image = (rows) => ({ kind: "image", rows });
export const circle = (op, ...args) => ({ kind: "circle", op, args });
const signatures = {
  add: ["num", "num"],
  sub: ["num", "num"],
  mul: ["num", "num"],
  pow: ["num", "num"],
  rotate: ["image", "num"],
  recolor: ["image", "color"],
  beside: ["image", "image"],
};
export const size = (t) =>
  t.kind === "circle" ? 1 + t.args.reduce((n, a) => n + size(a), 0) : 0;
export const depth = (t) =>
  t.kind === "circle" ? 1 + Math.max(0, ...t.args.map(depth)) : 0;
export const key = (t) => JSON.stringify(t);
export const equal = (a, b) => key(a) === key(b);
export function show(t) {
  if (t.kind === "circle")
    return `<${t.op}${t.args.length ? " " : ""}${t.args.map(show).join(" ")}>`;
  if (t.kind === "num")
    return t.value.length > 70
      ? `${t.value.slice(0, 28)}… (${t.value.length} digits)`
      : t.value;
  if (t.kind === "image")
    return `image(${t.rows.map((r) => r.join("")).join("/")})`;
  return t.name;
}
export function validate(t, level = 0, counter = { n: 0 }) {
  if (++counter.n > 255 || level > 24)
    throw Error("Use at most 255 terms and nesting depth 24.");
  if (!t || typeof t !== "object")
    throw Error("A term must be a typed object.");
  if (t.kind === "num") {
    if (typeof t.value !== "string" || !/^-?\d{1,20000}$/.test(t.value))
      throw Error(
        "Numbers must be decimal integer strings (at most 20,000 digits).",
      );
    return num(t.value);
  }
  if (t.kind === "sym" || t.kind === "color") {
    if (
      typeof t.name !== "string" ||
      !/^[A-Za-z][A-Za-z0-9_-]{0,31}$/.test(t.name)
    )
      throw Error(
        "Names must contain letters, digits, underscores or hyphens.",
      );
    return { kind: t.kind, name: t.name };
  }
  if (t.kind === "image") {
    if (
      !Array.isArray(t.rows) ||
      !t.rows.length ||
      t.rows.length > 16 ||
      !Array.isArray(t.rows[0]) ||
      !t.rows[0].length ||
      t.rows[0].length > 16 ||
      !t.rows.every(
        (r) =>
          Array.isArray(r) &&
          r.length === t.rows[0].length &&
          r.every(
            (p) =>
              typeof p === "string" &&
              /^(\.|[A-Za-z][A-Za-z0-9_-]{0,31})$/.test(p),
          ),
      )
    )
      throw Error(
        "Images must be rectangular, nonempty grids no larger than 16 × 16.",
      );
    return image(t.rows.map((r) => [...r]));
  }
  if (t.kind === "circle") {
    if (
      typeof t.op !== "string" ||
      !/^[A-Za-z][A-Za-z0-9_-]{0,31}$/.test(t.op) ||
      !Array.isArray(t.args) ||
      t.args.length > 8
    )
      throw Error("Invalid circle operator or operands.");
    return circle(t.op, ...t.args.map((a) => validate(a, level + 1, counter)));
  }
  throw Error("Unknown term kind.");
}
export function reason(t) {
  if (t.kind !== "circle") return "Already a value.";
  const sig = signatures[t.op];
  if (!sig) return `Unknown operator: ${t.op}.`;
  if (t.args.length !== sig.length)
    return `${t.op} needs ${sig.length} operands.`;
  if (t.args.some((a) => a.kind === "circle"))
    return "Waiting for an inner circle.";
  if (t.args.some((a, i) => a.kind !== sig[i]))
    return `Requires ${sig.join(" × ")}; symbols need substitution.`;
  if (t.op === "pow" && BigInt(t.args[1].value) < 0n)
    return "Negative exponents are outside the integer carrier.";
  if (t.op === "rotate" && BigInt(t.args[1].value) % 90n !== 0n)
    return "Rotation requires a multiple of 90 degrees.";
  if (t.op === "beside" && t.args[0].rows.length !== t.args[1].rows.length)
    return "Images must have equal heights.";
  return null;
}
export class BudgetError extends Error {}
export function evaluate(t) {
  const why = reason(t);
  if (why) throw Error(why);
  const [a, b] = t.args;
  if (["add", "sub", "mul", "pow"].includes(t.op)) {
    const x = BigInt(a.value),
      y = BigInt(b.value);
    let v;
    if (t.op === "add") v = x + y;
    if (t.op === "sub") v = x - y;
    if (t.op === "mul") v = x * y;
    if (t.op === "pow") {
      // A resource boundary, never a semantic refusal. Allow the five-two tower.
      if (
        y > 100000n ||
        (x !== 0n &&
          x !== 1n &&
          x !== -1n &&
          BigInt((x < 0n ? -x : x).toString(2).length) * y > 140000n)
      )
        throw new BudgetError(
          "Power exceeds the browser arithmetic budget; the circle is admissible but has not been evaluated.",
        );
      v = x ** y;
    }
    if (v.toString().length > 20000)
      throw new BudgetError("Result exceeds the 20,000-digit display budget.");
    return num(v);
  }
  if (t.op === "recolor")
    return image(a.rows.map((r) => r.map((p) => (p === "." ? "." : b.name))));
  if (t.op === "beside") {
    if (a.rows[0].length + b.rows[0].length > 16)
      throw new BudgetError("Result exceeds the 16-column image budget.");
    return image(a.rows.map((r, i) => [...r, ...b.rows[i]]));
  }
  let rows = a.rows.map((r) => [...r]),
    turns = Number((((BigInt(b.value) / 90n) % 4n) + 4n) % 4n);
  while (turns--) rows = rows[0].map((_, i) => rows.map((r) => r[i]).reverse());
  return image(rows);
}
export function ready(t, path = []) {
  if (t.kind !== "circle") return [];
  const children = t.args.flatMap((a, i) => ready(a, [...path, i]));
  return reason(t) === null ? [...children, { path, term: t }] : children;
}
export function at(t, path) {
  return path.reduce((a, i) => a.args[i], t);
}
export function replace(t, path, value) {
  if (!path.length) return value;
  const [i, ...rest] = path;
  return circle(
    t.op,
    ...t.args.map((a, j) => (j === i ? replace(a, rest, value) : a)),
  );
}
export function pop(t, path) {
  return replace(t, path, evaluate(at(t, path)));
}
export function parallel(t) {
  // Capture the frontier before reducing: parents newly enabled wait one round.
  const frontier = ready(t);
  return {
    term: frontier.reduce((out, r) => pop(out, r.path), t),
    pops: frontier.length,
  };
}
export function substitute(t, env) {
  if (t.kind === "sym") return env[t.name] ?? t;
  return t.kind === "circle"
    ? circle(t.op, ...t.args.map((a) => substitute(a, env)))
    : t;
}
export function symbols(t) {
  return [
    ...new Set(
      t.kind === "sym"
        ? [t.name]
        : t.kind === "circle"
          ? t.args.flatMap(symbols)
          : [],
    ),
  ].sort();
}
export function normalize(t, strategy = "left") {
  let pops = 0,
    rounds = 0;
  while (ready(t).length) {
    if (strategy === "parallel") {
      const next = parallel(t);
      t = next.term;
      pops += next.pops;
    } else {
      const rs = ready(t);
      t = pop(t, rs[strategy === "right" ? rs.length - 1 : 0].path);
      pops++;
    }
    rounds++;
    if (rounds > 255) throw new BudgetError("Reduction budget reached.");
  }
  return { term: t, pops, rounds };
}
export function explore(t, limit = 1200) {
  const queue = [t],
    seen = new Set([key(t)]),
    normalForms = new Map();
  let edges = 0;
  for (let i = 0; i < queue.length; i++) {
    const rs = ready(queue[i]);
    if (!rs.length) normalForms.set(key(queue[i]), queue[i]);
    for (const r of rs) {
      const next = pop(queue[i], r.path);
      edges++;
      if (!seen.has(key(next))) {
        if (seen.size >= limit)
          return {
            complete: false,
            states: seen.size,
            edges,
            normalForms: [...normalForms.values()],
          };
        seen.add(key(next));
        queue.push(next);
      }
    }
  }
  return {
    complete: true,
    states: seen.size,
    edges,
    normalForms: [...normalForms.values()],
  };
}
const N = num,
  S = sym,
  C = circle;
const I = image([
  ["x", ".", "x"],
  [".", "x", "."],
]);
export const presets = [
  {
    name: "Two independent pops",
    chapter: "1, 4",
    note: "Choose either multiplication. Both choices leave the other ready and converge to 26.",
    term: C("add", C("mul", N(2), N(3)), C("mul", N(4), N(5))),
  },
  {
    name: "Grouping changes the answer",
    chapter: "1",
    note: "The explicit grouping 2 + (3 × 4) yields 14. Edit the JSON to move add inside mul and obtain 20.",
    term: C("add", N(2), C("mul", N(3), N(4))),
  },
  {
    name: "Seven circles, three rounds",
    chapter: "4",
    note: "A balanced tree takes seven individual pops, or three simultaneous rounds. Rounds are logical depth, not a wall-clock benchmark.",
    term: C(
      "add",
      C("add", C("add", N(1), N(1)), C("add", N(1), N(1))),
      C("add", C("add", N(1), N(1)), C("add", N(1), N(1))),
    ),
  },
  {
    name: "A symbol waits",
    chapter: "6",
    note: "The numerical multiplication can pop now. Supply x = 5 to enable the remaining addition.",
    term: C("add", S("x"), C("mul", N(3), N(4))),
  },
  {
    name: "An inert integer expression",
    chapter: "5",
    note: "A negative exponent is outside this integer operator’s domain. No ready circles does not necessarily mean a value.",
    term: C("add", N(1), C("pow", N(2), N(-1))),
  },
  {
    name: "Right-nested tower",
    chapter: "4, 6",
    note: "Four twos, three circles, result 65536. Each pop counts once, although arithmetic costs differ.",
    term: C("pow", N(2), C("pow", N(2), C("pow", N(2), N(2)))),
  },
  {
    name: "Left-nested tower",
    chapter: "4",
    note: "The same four twos with different grouping give 256, again in three pops.",
    term: C("pow", C("pow", C("pow", N(2), N(2)), N(2)), N(2)),
  },
  {
    name: "Recolour, then rotate",
    chapter: "7",
    note: "Red replaces occupied cells; dots stay transparent. Rotation is clockwise and the original value is immutable.",
    term: C("rotate", C("recolor", I, color("red")), N(90)),
  },
  {
    name: "Compose two images",
    chapter: "7",
    note: "Independent image operations can pop in either order before beside combines their equal-height results.",
    term: C(
      "beside",
      C(
        "rotate",
        image([
          ["x", "y"],
          ["z", "."],
        ]),
        N(90),
      ),
      C(
        "recolor",
        image([
          ["x", "."],
          [".", "x"],
        ]),
        color("blue"),
      ),
    ),
  },
  {
    name: "Rotation waits for admissibility",
    chapter: "5, 7",
    note: "Forty-five degrees is outside the grid operator’s domain. The expression remains available for editing.",
    term: C("rotate", I, N(45)),
  },
];
