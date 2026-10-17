import {
  presets,
  ready,
  pop,
  parallel,
  size,
  depth,
  show,
  reason,
  substitute,
  symbols,
  num,
  normalize,
  explore,
  equal,
  validate,
} from "./engine.mjs";
const $ = (id) => document.getElementById(id);
const el = (tag, text, className) => {
  const e = document.createElement(tag);
  if (text !== undefined) e.textContent = text;
  if (className) e.className = className;
  return e;
};
let initial = presets[0].term,
  current = initial,
  example = presets[0],
  selected = 0,
  actions = [],
  undo = [],
  comparison = "";
let substitutionNames = null;
let substitutionDrafts = new Map();
function message(text = "") {
  $("message").textContent = text;
}
const pathName = (p) => (p.length ? p.map((i) => i + 1).join(".") : "root");
function load(term, meta) {
  initial = term;
  current = term;
  example = meta;
  selected = 0;
  actions = [];
  undo = [];
  comparison = "";
  substitutionNames = null;
  substitutionDrafts = new Map();
  $("source").value = JSON.stringify(term, null, 2);
  message();
  render();
}
function commit(term, label, pops = 0, rounds = 0, paths = []) {
  undo.push({ current, actions: [...actions], selected, comparison });
  actions.push({
    action: label,
    paths,
    input: current,
    output: term,
    pops,
    parallelRounds: rounds,
  });
  current = term;
  selected = 0;
  comparison = "";
  message();
  render();
}
function safely(fn) {
  try {
    fn();
  } catch (error) {
    message(error.message);
  }
}
function choosePop() {
  safely(() => {
    const r = ready(current)[selected];
    if (r)
      commit(
        pop(current, r.path),
        `Pop ${r.term.op} at ${pathName(r.path)}`,
        1,
        0,
        [r.path],
      );
  });
}
function oneRound() {
  safely(() => {
    const frontier = ready(current);
    if (!frontier.length) return;
    const next = parallel(current);
    commit(
      next.term,
      `Parallel round: ${next.pops} pops`,
      next.pops,
      1,
      frontier.map((r) => r.path),
    );
  });
}
function back() {
  const state = undo.pop();
  if (state) {
    ({ current, actions, selected, comparison } = state);
    message();
    render();
  }
}
const palette = {
  red: "#e87366",
  blue: "#79aef2",
  green: "#92ca79",
  yellow: "#f1d17b",
  x: "#c8dbbd",
  y: "#bd9ae5",
  z: "#e6b176",
};
function draw(t, path = []) {
  if (t.kind === "circle") {
    const rs = ready(current),
      index = rs.findIndex(
        (r) => JSON.stringify(r.path) === JSON.stringify(path),
      );
    const box = el(
      "div",
      undefined,
      `circle${index >= 0 ? " ready" : ""}${index === selected ? " selected" : ""}`,
    );
    box.append(el("span", t.op, "operator"));
    const args = el("div", undefined, "operands");
    t.args.forEach((a, i) => args.append(draw(a, [...path, i])));
    box.append(args);
    box.title = reason(t) ?? `Ready: ${t.op}. Select, then pop.`;
    if (index >= 0) {
      box.tabIndex = 0;
      box.setAttribute("role", "button");
      box.setAttribute(
        "aria-label",
        `Select ready ${t.op} at ${pathName(path)}`,
      );
      box.setAttribute("aria-pressed", String(index === selected));
      box.addEventListener("click", (event) => {
        event.stopPropagation();
        selected = index;
        render();
      });
      box.addEventListener("keydown", (event) => {
        if (event.key === " " || event.key === "Enter") {
          event.preventDefault();
          event.stopPropagation();
          selected = index;
          render();
          $("pop").focus();
        }
      });
    }
    return box;
  }
  if (t.kind === "image") {
    const grid = el("div", undefined, "pixel-image");
    grid.style.gridTemplateColumns = `repeat(${t.rows[0].length},1fr)`;
    grid.setAttribute("role", "img");
    grid.setAttribute(
      "aria-label",
      `Image: ${t.rows.map((r) => r.join(" ")).join("; ")}`,
    );
    t.rows.flat().forEach((p) => {
      const cell = el("span", undefined, "pixel");
      cell.style.background =
        p === "." ? "transparent" : (palette[p] ?? "#d4bd91");
      cell.title = p === "." ? "transparent" : p;
      grid.append(cell);
    });
    return grid;
  }
  const atom = el(
    "span",
    show(t),
    `atom${t.kind === "sym" ? " symbol" : ""}${t.kind === "num" && t.value.length > 70 ? " long-number" : ""}`,
  );
  if (t.kind === "color") atom.style.color = palette[t.name] ?? "#d4bd91";
  return atom;
}
function render() {
  const rs = ready(current);
  selected = Math.max(0, Math.min(selected, rs.length - 1));
  $("title").textContent = example.name;
  $("chapter").textContent = `BOOK CHAPTERS ${example.chapter}`;
  $("note").textContent = example.note;
  $("status").textContent =
    current.kind !== "circle"
      ? "Value"
      : rs.length
        ? "Ready to pop"
        : "Inert normal form";
  $("status").className =
    `badge${current.kind === "circle" && !rs.length ? " blocked" : ""}`;
  $("remaining").textContent = size(current);
  $("height").textContent = depth(current);
  $("pops").textContent = actions.reduce((n, a) => n + a.pops, 0);
  $("rounds").textContent = actions.reduce((n, a) => n + a.parallelRounds, 0);
  $("canvas").replaceChildren(draw(current));
  $("expression").textContent = show(current);
  $("pop").disabled = !rs.length;
  $("parallel").disabled = !rs.length;
  $("undo").disabled = !undo.length;
  $("selection").textContent = rs.length
    ? `${rs.length} ready ${rs.length === 1 ? "circle" : "circles"}. Selected ${rs[selected].term.op} at ${pathName(rs[selected].path)}.`
    : current.kind === "circle"
      ? reason(current)
      : "Evaluation has reached a value.";
  $("presets")
    .querySelectorAll("button")
    .forEach((b, i) => {
      b.classList.toggle("active", presets[i] === example);
      b.setAttribute("aria-pressed", String(presets[i] === example));
    });
  renderSubstitution();
  $("trace").replaceChildren();
  if (!actions.length)
    $("trace").append(el("li", "No actions yet. Choose a ready circle."));
  actions.forEach((a) => {
    const li = el("li");
    li.append(el("strong", a.action), el("code", show(a.output)));
    $("trace").append(li);
  });
  $("comparison").replaceChildren();
  if (comparison) $("comparison").append(comparison);
}
function renderSubstitution() {
  const names = symbols(current);
  const signature = JSON.stringify(names);
  if (signature === substitutionNames) return;
  substitutionNames = signature;
  $("substitution").replaceChildren();
  if (names.length) {
    $("substitution").append(
      el(
        "p",
        "Supply integer values for free symbols. Blank fields leave symbols unchanged.",
        "quiet",
      ),
    );
    const inputs = {};
    names.forEach((name) => {
      const label = el("label", `${name} =`);
      const input = el("input");
      input.inputMode = "numeric";
      input.value = substitutionDrafts.get(name) ?? "";
      input.addEventListener("input", () =>
        substitutionDrafts.set(name, input.value),
      );
      input.setAttribute("aria-label", `Integer value for ${name}`);
      inputs[name] = input;
      label.append(input);
      $("substitution").append(label);
    });
    const apply = el("button", "Substitute");
    apply.onclick = () =>
      safely(() => {
        const env = {};
        for (const [name, input] of Object.entries(inputs)) {
          const value = input.value.trim();
          if (value) {
            if (!/^-?\d{1,100}$/.test(value))
              throw Error("Enter integers of at most 100 digits.");
            env[name] = num(value);
          }
        }
        if (Object.keys(env).length)
          commit(
            substitute(current, env),
            `Substitute ${Object.entries(env)
              .map(([k, v]) => `${k} = ${v.value}`)
              .join(", ")}`,
          );
      });
    $("substitution").append(apply);
  }
}

presets.forEach((p) => {
  const button = el("button", p.name);
  button.onclick = () => load(p.term, p);
  $("presets").append(button);
});
$("pop").onclick = choosePop;
$("parallel").onclick = oneRound;
$("undo").onclick = back;
$("reset").onclick = () => load(initial, example);
$("load").onclick = () =>
  safely(() => {
    if ($("source").value.length > 100000)
      throw Error("Expression JSON exceeds the input budget.");
    load(validate(JSON.parse($("source").value)), {
      name: "Your expression",
      chapter: "1–7",
      note: "Select a ready circle, explore alternative orders, or supply a free symbol.",
    });
  });
$("compare").onclick = () =>
  safely(() => {
    const results = ["left", "right", "parallel"].map((strategy) => ({
      strategy,
      ...normalize(current, strategy),
    }));
    const graph = explore(current);
    const box = el("div");
    box.append(el("strong", "Comparison from the current expression"));
    const table = el("table");
    const head = el("thead");
    const hr = el("tr");
    ["Strategy", "Pops", "Rounds / steps", "Normal form"].forEach((s) =>
      hr.append(el("th", s)),
    );
    head.append(hr);
    table.append(head);
    const body = el("tbody");
    results.forEach((r) => {
      const row = el("tr");
      [r.strategy, r.pops, r.rounds, show(r.term)].forEach((v) =>
        row.append(el("td", v)),
      );
      body.append(row);
    });
    table.append(body);
    box.append(table);
    box.append(
      el(
        "p",
        `${results.every((r) => equal(r.term, results[0].term)) ? "All three strategies agree." : "The strategies disagree."} ${graph.complete ? "Complete bounded exploration" : "Exploration stopped at its state budget"}: ${graph.states} distinct expressions, ${graph.edges} transitions, ${graph.normalForms.length} ${graph.complete ? "" : "observed "}normal form(s). This checks this expression; it does not prove a general theorem.`,
      ),
    );
    comparison = box;
    render();
  });
$("export").onclick = () => {
  const data = {
    format: "spherepop-circle-experiment/1",
    semantics: "pure typed value-returning operators, book chapters 1–7",
    initial,
    actions,
    final: current,
  };
  const url = URL.createObjectURL(
    new Blob([JSON.stringify(data, null, 2)], { type: "application/json" }),
  );
  const a = el("a");
  a.href = url;
  a.download = "spherepop-circle-trace.json";
  a.click();
  setTimeout(() => URL.revokeObjectURL(url), 1000);
};
document.addEventListener("keydown", (event) => {
  if (
    event.ctrlKey ||
    event.metaKey ||
    event.altKey ||
    ["INPUT", "TEXTAREA", "SELECT"].includes(event.target.tagName) ||
    event.target.isContentEditable
  )
    return;
  // Preserve native activation on focused controls, but allow letter shortcuts.
  if (
    event.target.closest("button, a, summary, [role=button]") &&
    (event.key === "Enter" || event.key === " ")
  )
    return;
  const k = event.key.toLowerCase();
  if (["j", "k", "enter", "p", "u", "r"].includes(k)) {
    event.preventDefault();
    if (k === "j" || k === "k") {
      const n = ready(current).length;
      if (n) {
        selected = (selected + (k === "j" ? 1 : n - 1)) % n;
        render();
      }
    }
    if (k === "enter") choosePop();
    if (k === "p") oneRound();
    if (k === "u") back();
    if (k === "r") load(initial, example);
  }
});
load(initial, example);
