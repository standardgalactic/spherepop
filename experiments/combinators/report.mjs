// Turns generated/facts.json into LaTeX so that no number in the essay is typed by hand.
import fs from "node:fs";
const f = JSON.parse(fs.readFileSync(new URL("./generated/facts.json", import.meta.url), "utf8"));
const fmt = (n) => (typeof n === "string" && !/^\d+$/.test(n) ? n : String(n).replace(/\B(?=(\d{3})+(?!\d))/g, "{,}"));
const tt = (s) => "\\texttt{" + String(s).replace(/([\\{}_#%&$^~])/g, "\\$1") + "}";
const macros = [];
const def = (name, v) => macros.push(`\\newcommand{\\${name}}{${v}}`);
const pct = (a, b) => (100 * a / b).toFixed(1) + "\\%";

def("factEnumSeven", fmt(f.enumeration[7]));
def("factRedexes", fmt(f.sizeLaw.redexes));
def("factKRedexes", fmt(f.sizeLaw.kCount));
def("factSRedexes", fmt(f.sizeLaw.sCount));
def("factGrowing", fmt(f.sizeLaw.growing));
def("factGrowBefore", tt(f.growExample.before));
def("factGrowAfter", tt(f.growExample.after));
def("factGrowSizeBefore", f.growExample.sizeBefore);
def("factGrowSizeAfter", f.growExample.sizeAfter);
def("factStepsToForty", fmt(f.omega.stepsTo40));
def("factTriTerms", fmt(f.triangle.terms));
def("factTriPairs", fmt(f.triangle.pairs));
def("factTriNontrivial", fmt(f.triangle.nontrivial));
def("factDiamondTerm", tt(f.diamond.first.term));
def("factDiamondLeft", tt(f.diamond.first.left));
def("factDiamondRight", tt(f.diamond.first.right));
def("factDiamondFailing", fmt(f.diamond.failing[6]));
def("factUniqueNF", fmt(f.uniqueNF.complete));
def("factNormTerms", fmt(f.normalization.terms));
def("factNormLoNormal", fmt(f.normalization.loNormal));
def("factNormNone", fmt(f.normalization.nonNormalAll));
def("factInnermostMiss", fmt(f.normalization.innermostMisses));
def("factRoMiss", fmt(f.normalization.rightOutermostMisses));
def("factSmallestNonNormalA", tt(f.normalization.smallestNonNormal[0]));
def("factSmallestNonNormalB", tt(f.normalization.smallestNonNormal[1]));
def("factOptTerms", fmt(f.optimality.terms));
def("factDiffTerms", fmt(f.differential.terms));
def("factDiffComparisons", fmt(f.differential.comparisons));
def("factDiffCycle", fmt(f.differential.tally.cycle));
def("factDiffBudget", fmt(f.differential.tally.budget));
def("factDiffSize", fmt(f.differential.tally.size));
def("factMachineCompared", fmt(f.machineAgreement.compared));
def("factSharedLess", fmt(f.machineAgreement.sharedLess));
def("factAbsCases", fmt(f.abstraction.cases));
def("factAbsTerms", fmt(f.abstraction.terms));
def("factAbsSteps", fmt(f.abstraction.totalSteps));
def("factEtaChecked", fmt(f.eta.checked));
def("factEtaSmaller", fmt(f.eta.smaller));
def("factSubstChecked", fmt(f.substLemma.checked));
def("factWeakSim", fmt(f.weakSim.simulated));
def("factWeakContractions", fmt(f.weakSim.contractions));
def("factUnderTerms", fmt(f.underBinder.terms));
def("factUnderNotJoinable", fmt(f.underBinder.notJoinable));
def("factUnderJoinable", fmt(f.underBinder.appliedJoinable));
def("factUnderDirect", fmt(f.underBinder.directlyJoinable));
def("factUnderNever", fmt(f.underBinder.neverJoinable));
def("factUnderNeverDistinct", fmt(f.underBinder.neverJoinableBothNormalDistinct));
def("factUnderUndecided", fmt(f.underBinder.undecided));
def("factChurchCases", fmt(f.church.cases));
def("factMachineRuns", fmt(f.machines.runs));
def("factDivergencePrefix", fmt(f.divergence.prefix));
def("factPureTrees", fmt(f.pure.trees));
def("factBalancedOrderings", fmt(f.pure.balancedOrderings));
def("factBalancedIdeals", fmt(f.pure.balancedIdeals));
def("factPolyPairs", fmt(f.decompose.pairs));
def("factDecompM", f.decompose.example.m);
def("factDecompD", f.decompose.example.d);
def("factDecompEstimate", fmt(f.decompose.example.estimate));
def("factDecompCorrection", String(f.decompose.example.correction).replace("-", "{-}"));
def("factDecompValue", fmt(f.decompose.example.value));
def("factDecompPopsOriginal", f.decompose.example.pops.original);
def("factDecompPopsSplit", f.decompose.example.pops.split);
def("factDecompPopsSquares", f.decompose.example.pops.squares);
def("factLoOptPct", pct(f.optimality.lo, f.optimality.terms));
def("factRoOptPct", pct(f.optimality.ro, f.optimality.terms));
def("factLiOptPct", pct(f.optimality.li, f.optimality.terms));
def("factLiWorse", fmt(f.optimality.liWorse));
def("factLoWorse", fmt(f.optimality.loWorse));
def("factLoExample", tt(f.optimality.loExample.term));
def("factLoExampleLo", f.optimality.loExample.lo);
def("factLoExampleBest", f.optimality.loExample.best);
def("factLiExample", tt(f.optimality.liExample.term));
def("factLiExampleLi", f.optimality.liExample.li);
def("factLiExampleBest", f.optimality.liExample.best);
const gr = f.growth;
def("factMSizeStart", fmt(gr[0].size));
def("factMDagStart", gr[0].dag);
def("factMStepEnd", gr[gr.length - 1].step);
def("factMSizeEnd", fmt(gr[gr.length - 1].size));
def("factMDagEnd", gr[gr.length - 1].dag);
const sh = f.sharing;
def("factShareRatioFirst", (sh[0].treeSteps / sh[0].sharedSteps).toFixed(1));
def("factShareRatioLast", (sh[sh.length - 1].treeSteps / sh[sh.length - 1].sharedSteps).toFixed(1));
def("factShareNFirst", sh[0].n);
def("factShareNLast", sh[sh.length - 1].n);
def("factBellmanGraphs", fmt(f.bellman.acyclicGraphs));
def("factGroupingExpressions", fmt(f.grouping.expressions));
fs.writeFileSync(new URL("./generated/facts.tex", import.meta.url), "% generated by report.mjs; do not edit\n" + macros.join("\n") + "\n");

const rows = (arr) => arr.join(" \\\\\n") + " \\\\\n";
const T = [];
const table = (name, spec, head, body) => T.push(`\\newcommand{\\${name}}{%\n\\begin{tabular}{${spec}}\n\\toprule\n${head} \\\\\n\\midrule\n${rows(body)}\\bottomrule\n\\end{tabular}}`);

table("tableEnum", "rr", "$n$ & words of $n$ leaves over $\\{S,K,x\\}$",
  Object.entries(f.enumeration).map(([n, c]) => `${n} & ${fmt(c)}`));
table("tableOmega", "rrr", "$n$ & leftmost-outermost steps & circles",
  f.omega.rows.map((r) => `${r.n} & ${r.steps} & ${r.size}`));
const st = (o) => (o.status === "cycle" ? `cycle, period ${o.period ?? o.steps}` : o.status === "normal" ? `normal form in ${o.steps} step${o.steps === 1 ? "" : "s"}` : `no normal form in ${o.steps} steps`);
table("tableStrategies", "lll", "strategy & $\\Omega$ & $K\\,a\\,\\Omega$",
  ["lo", "li", "ri", "ro"].map((s) => `${s} & ${st(f.omegaStrategies[s])} & ${st(f.kOmega[s])}`));
table("tableGrowth", "rrr", "step & circles & distinct nodes",
  f.growth.map((r) => `${r.step} & ${fmt(r.size)} & ${r.dag}`));
table("tableOptimal", "lrr", "strategy & optimal & strictly worse",
  ["lo", "li", "ri", "ro"].map((s) => `${s} & ${fmt(f.optimality[s])} (${pct(f.optimality[s], f.optimality.terms)}) & ${fmt(f.optimality[s + "Worse"])}`));
table("tableAbs", "rrr", "$n$ & plain leaves & eta leaves",
  f.abstractionGrowth.map((r) => `${r.n} & ${r.plain} & ${r.eta}`));
const names = ["add", "double", "monus", "mult", "parity"];
table("tableSizes", "lrrrrr", "program & states & registers & circles & shared nodes & circles (eta)",
  names.map((n) => { const r = f["size_" + n]; return `${tt(n)} & ${r.states} & ${r.registers} & ${fmt(r.circles)} & ${r.dag} & ${fmt(r.circlesEta)}`; }));
table("tableMachines", "llrrrr", "program & input & machine & lambda & circle & output",
  f.machines.table.map((r) => `${tt(r.name)} & ${r.input} & ${fmt(r.machineSteps)} & ${fmt(r.lambdaSteps)} & ${fmt(r.circleSteps)} & ${r.output}`));
table("tableSharing", "rrrrr", "$n$ & tree steps & shared steps & ratio & output circles",
  f.sharing.map((r) => `${r.n} & ${fmt(r.treeSteps)} & ${fmt(r.sharedSteps)} & ${(r.treeSteps / r.sharedSteps).toFixed(2)} & ${fmt(r.nfCircles)}`));
const g = f.grouping;
table("tableGrouping", "ll", "expression & values (number of parenthesizations)",
  [`$2+3\\times4$ & ${g.sumProduct.map(([v, c]) => `${v} (${c})`).join(", ")}`,
   `$2^{2^{2^2}}$ read freely & ${g.tower.map(([v, c]) => `${v} (${c})`).join(", ")}`,
   `$8-3-2$ & ${g.minus.map(([v, c]) => `${v} (${c})`).join(", ")}`,
   `Catalan numbers $C_1,\\dots,C_8$ & ${g.catalan.join(", ")}`]);
fs.writeFileSync(new URL("./generated/tables.tex", import.meta.url), "% generated by report.mjs; do not edit\n" + T.join("\n\n") + "\n");
console.log("wrote facts.tex and tables.tex:", macros.length, "macros,", T.length, "tables");
