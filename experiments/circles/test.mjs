import assert from "node:assert/strict";
import { spawnSync } from "node:child_process";
import { fileURLToPath } from "node:url";
import {
  num as N,
  sym as S,
  color,
  image,
  circle as C,
  presets,
  normalize,
  ready,
  pop,
  parallel,
  size,
  depth,
  equal,
  key,
  substitute,
  explore,
  validate,
  BudgetError,
} from "./engine.mjs";

let checks = 0;
function check(name, fn) {
  fn();
  checks++;
  console.log(`PASS ${name}`);
}
function freeze(t) {
  Object.freeze(t);
  if (t.kind === "circle") t.args.forEach(freeze);
  for (const v of Object.values(t))
    if (Array.isArray(v)) {
      Object.freeze(v);
      v.filter(Array.isArray).forEach(Object.freeze);
    }
  return t;
}
const cases = presets.map((p) => ({ term: p.term }));
let seed = 20261017;
function random(n) {
  seed = (Math.imul(seed, 1664525) + 1013904223) >>> 0;
  return seed % n;
}
function generated(d) {
  if (!d || random(3) === 0) return random(5) === 0 ? S("x") : N(random(9) - 4);
  return C(
    ["add", "sub", "mul"][random(3)],
    generated(d - 1),
    generated(d - 1),
  );
}
for (let i = 0; i < 160; i++) {
  const term = generated(3);
  cases.push({ term }, { term, env: { x: N(random(7)) } });
}
for (const term of [
  C("pow", N(0), N(0)),
  C("pow", N(2), N(-1)),
  C("add", color("red"), N(2)),
  C("add", N(2)),
  C("unknown", N(1)),
  C(
    "rotate",
    image([
      ["a", "b", "c"],
      ["d", "e", "f"],
    ]),
    N(-90),
  ),
  C("beside", image([["a"]]), image([["b"], ["c"]])),
  C("pow", N(2), N(65536)),
])
  cases.push({ term });
const oracle = spawnSync(
  "python3",
  [fileURLToPath(new URL("oracle.py", import.meta.url))],
  {
    input: JSON.stringify(cases),
    encoding: "utf8",
    maxBuffer: 20 * 1024 * 1024,
  },
);
assert.equal(oracle.status, 0, oracle.stderr);
const expected = JSON.parse(oracle.stdout);
check(
  `${cases.length} cases agree with the book interpreter (result, step, pop count, size)`,
  () =>
    cases.forEach((c, i) => {
      const term = c.env ? substitute(c.term, c.env) : c.term;
      const before = key(term);
      freeze(term);
      assert.deepEqual(normalize(term).term, expected[i].term, `case ${i}`);
      assert.equal(normalize(term).pops, expected[i].pops, `pop count ${i}`);
      assert.equal(size(term), expected[i].size);
      const rs = ready(term);
      assert.deepEqual(
        rs.length ? pop(term, rs[0].path) : null,
        expected[i].step,
        `step ${i}`,
      );
      assert.equal(key(term), before, "input mutated");
    }),
);
check(
  "every ready pop removes exactly one circle, including partial terms",
  () =>
    cases
      .slice(0, 330)
      .forEach((c) =>
        ready(c.term).forEach((r) =>
          assert.equal(size(pop(c.term, r.path)), size(c.term) - 1),
        ),
      ),
);
check("independent pops form diamonds", () =>
  cases.slice(0, 330).forEach((c) => {
    const rs = ready(c.term);
    for (let i = 0; i < rs.length; i++)
      for (let j = i + 1; j < rs.length; j++)
        assert(
          equal(
            pop(pop(c.term, rs[i].path), rs[j].path),
            pop(pop(c.term, rs[j].path), rs[i].path),
          ),
        );
  }),
);
check("left, right and parallel strategies agree on values and work", () =>
  cases.slice(0, 330).forEach((c) => {
    const a = normalize(c.term),
      b = normalize(c.term, "right"),
      p = normalize(c.term, "parallel");
    assert(equal(a.term, b.term));
    assert(equal(a.term, p.term));
    assert.equal(a.pops, b.pops);
    assert.equal(a.pops, p.pops);
    assert.equal(a.pops, size(c.term) - size(a.term));
  }),
);
check("balanced expression: seven pops, three rounds, value eight", () => {
  const t = presets[2].term,
    r = normalize(t, "parallel");
  assert.deepEqual(r, { term: N(8), pops: 7, rounds: 3 });
  assert.equal(r.rounds, depth(t));
  assert.equal(size(parallel(t).term), 3);
});
check(
  "substitution enables inert terms and commutes with an existing pop",
  () => {
    const t = C("add", S("x"), C("mul", N(3), N(4))),
      env = { x: N(5) };
    assert(equal(substitute(pop(t, [1]), env), pop(substitute(t, env), [1])));
    assert.deepEqual(normalize(substitute(t, env)), {
      term: N(17),
      pops: 2,
      rounds: 2,
    });
    assert.equal(ready(C("add", S("x"), N(1))).length, 0);
  },
);
check("grouping changes values without changing the three-pop count", () => {
  assert.equal(normalize(presets[5].term).term.value, "65536");
  assert.equal(normalize(presets[6].term).term.value, "256");
  assert.equal(normalize(presets[5].term).pops, 3);
  assert.equal(normalize(presets[6].term).pops, 3);
});
check(
  "image rotation orientation, four turns, transparency and persistence",
  () => {
    const i = image([
        ["a", "b", "c"],
        ["d", "e", "f"],
      ]),
      before = key(i);
    assert.deepEqual(
      normalize(C("rotate", i, N(90))).term,
      image([
        ["d", "a"],
        ["e", "b"],
        ["f", "c"],
      ]),
    );
    assert(equal(normalize(C("rotate", i, N(360))).term, i));
    const j = image([["x", "."]]);
    assert.deepEqual(
      normalize(C("recolor", j, color("red"))).term,
      image([["red", "."]]),
    );
    assert.equal(key(i), before);
  },
);
check("bounded exploration reports completeness honestly", () => {
  const r = explore(presets[0].term);
  assert.equal(r.complete, true);
  assert.equal(r.states, 5);
  assert.equal(r.normalForms.length, 1);
  assert(equal(r.normalForms[0], N(26)));
  assert.equal(explore(presets[2].term, 2).complete, false);
});
check("resource limits are distinguishable from inadmissibility", () => {
  const t = C("pow", N(2), N(100001));
  assert.equal(ready(t).length, 1);
  assert.throws(() => normalize(t), BudgetError);
});
check("typed input rejects malformed values without executing source", () => {
  assert.throws(() => validate({ kind: "num", value: "1.5" }));
  assert.throws(() => validate({ kind: "num", value: true }));
  assert.throws(() => validate(image([["a"], ["a", "b"]])));
  assert.throws(() => validate(S("<script>")));
  assert.deepEqual(validate(presets[0].term), presets[0].term);
});
console.log(
  `\n${checks} verification groups passed; ${cases.length} Python-reference comparisons.`,
);
