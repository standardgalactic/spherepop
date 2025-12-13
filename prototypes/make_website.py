import os, textwrap, zipfile, pathlib, json, re, datetime
base = "/mnt/data/spherepop_site"
os.makedirs(base, exist_ok=True)

site_title = "Spherepop OS"
nav = [
    ("Home", "index.html"),
    ("What Is Spherepop OS?", "what.html"),
    ("Core Principles", "principles.html"),
    ("Kernel Semantics", "kernel.html"),
    ("Time, Replay, Speculation", "time.html"),
    ("Views & Geometry", "views.html"),
    ("Collaboration Model", "collaboration.html"),
    ("Spherepop Calculus", "calculus.html"),
    ("Utilities Roadmap", "utilities.html"),
    ("Formal Spec", "spec.html"),
]

def nav_html(active_href):
    items=[]
    for label, href in nav:
        cls = "active" if href==active_href else ""
        items.append(f'<a class="navlink {cls}" href="{href}">{label}</a>')
    return "\n".join(items)

base_css = r"""
:root{
  --bg:#0b0f0c;
  --panel:#0f1712;
  --text:#d6ffe1;
  --muted:#9cc7aa;
  --accent:#53ff9a;
  --accent2:#66b2ff;
  --border:#1c2b22;
  --shadow: 0 10px 30px rgba(0,0,0,.45);
  --mono: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, "Liberation Mono", "Courier New", monospace;
  --sans: ui-sans-serif, system-ui, -apple-system, Segoe UI, Roboto, Helvetica, Arial, "Apple Color Emoji","Segoe UI Emoji";
}
*{box-sizing:border-box}
html,body{height:100%}
body{
  margin:0;
  font-family: var(--sans);
  background: radial-gradient(1200px 700px at 20% 10%, rgba(83,255,154,.10), transparent 60%),
              radial-gradient(900px 600px at 80% 30%, rgba(102,178,255,.09), transparent 55%),
              var(--bg);
  color: var(--text);
  line-height: 1.55;
}
a{color:var(--accent); text-decoration:none}
a:hover{text-decoration:underline}
header{
  position:sticky; top:0;
  backdrop-filter: blur(10px);
  background: rgba(11,15,12,.72);
  border-bottom:1px solid var(--border);
  z-index:50;
}
.container{max-width:1100px; margin:0 auto; padding: 0 18px}
.topbar{
  display:flex; align-items:center; justify-content:space-between;
  padding: 14px 0;
  gap:14px;
}
.brand{
  display:flex; flex-direction:column; gap:2px;
}
.brand .title{
  font-weight:750; letter-spacing:.2px;
}
.brand .subtitle{
  font-size: 12.5px; color: var(--muted);
}
nav{
  display:flex; flex-wrap:wrap;
  gap:10px;
  justify-content:flex-end;
}
.navlink{
  padding: 6px 10px;
  border:1px solid transparent;
  border-radius: 10px;
  font-size: 13px;
  color: var(--text);
  opacity:.9;
}
.navlink:hover{border-color: var(--border); text-decoration:none}
.navlink.active{
  border-color: rgba(83,255,154,.35);
  background: rgba(83,255,154,.08);
  color: var(--accent);
  opacity:1;
}
main{padding: 34px 0 60px}
.hero{
  border:1px solid var(--border);
  border-radius: 18px;
  background: linear-gradient(180deg, rgba(83,255,154,.08), rgba(15,23,18,.72));
  box-shadow: var(--shadow);
  padding: 26px;
}
.hero h1{margin:0 0 8px; font-size: 34px; line-height:1.15}
.hero p{margin:8px 0; color: var(--muted); max-width: 78ch}
.badges{display:flex; flex-wrap:wrap; gap:10px; margin-top: 14px}
.badge{
  font-family: var(--mono);
  font-size: 12px;
  padding: 6px 10px;
  border-radius: 999px;
  border:1px solid rgba(214,255,225,.16);
  background: rgba(11,15,12,.35);
  color: var(--text);
}
.grid{
  display:grid;
  grid-template-columns: repeat(12, 1fr);
  gap:16px;
  margin-top: 18px;
}
.card{
  grid-column: span 6;
  border:1px solid var(--border);
  border-radius: 16px;
  background: rgba(15,23,18,.72);
  box-shadow: var(--shadow);
  padding: 18px;
}
.card h2{margin:0 0 8px; font-size: 18px}
.card p{margin:0; color: var(--muted)}
.card .cta{margin-top: 12px; display:inline-block; font-family:var(--mono); font-size: 12.5px}
@media (max-width: 820px){
  .card{grid-column: span 12}
  nav{justify-content:flex-start}
  .topbar{flex-direction:column; align-items:flex-start}
}
h2{margin-top: 28px; font-size: 22px}
h3{margin-top: 22px; font-size: 17px}
section{margin-top: 18px}
.prose{
  border:1px solid var(--border);
  border-radius: 18px;
  background: rgba(15,23,18,.72);
  box-shadow: var(--shadow);
  padding: 22px;
}
.prose p{color: var(--muted)}
.prose ul{margin: 8px 0 0 18px; color: var(--muted)}
.prose li{margin: 6px 0}
.kbd{
  font-family: var(--mono);
  font-size: 12px;
  padding: 2px 6px;
  border: 1px solid rgba(214,255,225,.16);
  border-bottom-color: rgba(214,255,225,.28);
  border-radius: 8px;
  color: var(--text);
  background: rgba(11,15,12,.4);
}
.codeblock{
  font-family: var(--mono);
  font-size: 13px;
  border:1px solid rgba(214,255,225,.12);
  background: rgba(11,15,12,.55);
  border-radius: 14px;
  padding: 14px;
  overflow:auto;
  color: #e8fff0;
}
footer{
  border-top: 1px solid var(--border);
  padding: 18px 0 34px;
  color: var(--muted);
  font-size: 12.5px;
}
small.muted{color: var(--muted)}
hr.sep{border:0; border-top:1px solid var(--border); margin: 22px 0}
"""

def page_template(title, active, body_html, desc=None):
    desc = desc or "Spherepop OS — deterministic, log-authoritative semantic operating system."
    return f"""<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8"/>
  <meta name="viewport" content="width=device-width,initial-scale=1"/>
  <meta name="description" content="{desc}"/>
  <title>{title} — {site_title}</title>
  <link rel="stylesheet" href="styles.css"/>
</head>
<body>
<header>
  <div class="container">
    <div class="topbar">
      <div class="brand">
        <div class="title">{site_title}</div>
        <div class="subtitle">Deterministic semantic operating system • append-only authority • non-interfering views</div>
      </div>
      <nav>
        {nav_html(active)}
      </nav>
    </div>
  </div>
</header>

<main class="container">
{body_html}
</main>

<footer>
  <div class="container">
    <div>© {datetime.date.today().year} Flyxion • Spherepop OS site starter</div>
    <div><small class="muted">Built as static HTML/CSS so it can be hosted anywhere (GitHub Pages, Netlify, etc.).</small></div>
  </div>
</footer>
</body>
</html>
"""

# Build pages
index_body = """
<section class="hero">
  <h1>Spherepop OS</h1>
  <p>
    A semantic operating system whose primary abstraction is not the process, file, or thread, but a
    <b>deterministic, append-only relational event substrate</b>.
  </p>
  <p>
    Authoritative state transitions live only in the log. Everything else—snapshots, diffs, layouts, speculative branches—
    is derived and non-causal.
  </p>
  <div class="badges">
    <div class="badge">append-only authority</div>
    <div class="badge">total causal order</div>
    <div class="badge">deterministic replay</div>
    <div class="badge">view–cause separation</div>
    <div class="badge">semantic primacy</div>
  </div>
</section>

<div class="grid">
  <div class="card">
    <h2>Start here: what is Spherepop OS?</h2>
    <p>Short, rigorous explanation of the system and what it replaces in conventional OS design.</p>
    <a class="cta" href="what.html">open →</a>
  </div>
  <div class="card">
    <h2>Core invariants</h2>
    <p>Determinism, total order, semantic primacy, and the strict firewall between cause and view.</p>
    <a class="cta" href="principles.html">open →</a>
  </div>
  <div class="card">
    <h2>Kernel semantics</h2>
    <p>POP / MERGE / LINK / UNLINK / COLLAPSE / SETMETA as pure event interpretation.</p>
    <a class="cta" href="kernel.html">open →</a>
  </div>
  <div class="card">
    <h2>Spherepop calculus</h2>
    <p>How the OS substrate supports the calculus: equivalence, canonicalization, and semantic rewrites.</p>
    <a class="cta" href="calculus.html">open →</a>
  </div>
</div>

<section class="prose">
  <h2>One-sentence thesis</h2>
  <p>
    Spherepop OS treats <b>time-ordered semantic events</b> as the only authority, and forces everything else to be a
    <b>functorial view</b> that cannot interfere with the causal substrate.
  </p>
  <hr class="sep"/>
  <p>
    If you want the full formal document, see <a href="spec.html">Formal Spec</a>.
  </p>
</section>
"""

what_body = """
<section class="prose">
  <h2>What is Spherepop OS?</h2>
  <p>
    Spherepop OS is a semantic operating system designed around <b>semantic time</b>.
    It replaces mutable global state with a single append-only, totally ordered event log,
    interpreted by a deterministic kernel.
  </p>

  <h3>What it replaces</h3>
  <ul>
    <li><b>Processes / threads</b> as “primary reality” → replaced by <b>events</b> as authority</li>
    <li><b>Mutable object tables</b> → replaced by <b>replay-derived ontology</b></li>
    <li><b>Side effects as default</b> → replaced by <b>observational purity</b></li>
  </ul>

  <h3>What it enables</h3>
  <ul>
    <li>Exact deterministic replay from the authoritative log</li>
    <li>Late joiners: observers can join at any time with snapshots + diffs</li>
    <li>Seekable history: derive snapshots for any past event ID (EID)</li>
    <li>Speculative branches as local overlay logs, without timeline pollution</li>
  </ul>

  <hr class="sep"/>

  <p>
    Spherepop OS intentionally sits between an operating system kernel, a distributed database,
    and a semantic theory of computation.
  </p>
</section>
"""

principles_body = """
<section class="prose">
  <h2>Core principles</h2>
  <p>
    The design is constrained by a small set of invariants. These are non-negotiable because
    they guarantee replayability, collaboration, and introspection.
  </p>

  <h3>1) Append-only authority</h3>
  <p>
    All authoritative state transitions are recorded as events in a single append-only log.
    No other structure is allowed to introduce or modify semantic state.
  </p>

  <h3>2) Total causal order</h3>
  <p>
    Every committed event has a strict position (EID) assigned by a single sequencer.
    This yields a global, unambiguous ordering of state transitions.
  </p>

  <h3>3) Deterministic replay</h3>
  <p>
    Kernel transitions are pure functions of (prior state, next event).
    Therefore a log prefix uniquely determines the resulting state.
  </p>

  <h3>4) View–cause separation</h3>
  <p>
    Layout, visualization, snapshots, and diffs are derived views.
    They must be non-interfering: they cannot feed back into authoritative state.
  </p>

  <h3>5) Semantic primacy</h3>
  <p>
    Objects have no intrinsic meaning; meaning is induced by relations, equivalence,
    and the event history that produced them.
  </p>
</section>
"""

kernel_body = """
<section class="prose">
  <h2>Kernel semantics</h2>
  <p>
    The Spherepop kernel is a minimal deterministic interpreter of events.
    It maintains derived caches (equivalence classes, normalized relations, metadata maps),
    but these structures are never authoritative.
  </p>

  <h3>Kernel state (conceptual)</h3>
  <div class="codeblock">
σ = (O, U, R, M)
O: object identifiers
U: union–find parent map (equivalence)
R: typed relations (representative-normalized)
M: non-causal metadata
  </div>

  <h3>Event primitives</h3>
  <ul>
    <li><b>POP(o)</b>: create object handle</li>
    <li><b>MERGE(o1,o2)</b>: induce equivalence (union–find)</li>
    <li><b>LINK(o1,o2,t)</b>: create typed relation (normalized to representatives)</li>
    <li><b>UNLINK(o1,o2,t)</b>: remove typed relation</li>
    <li><b>COLLAPSE(S, or)</b>: bulk equivalence of a region S into representative or</li>
    <li><b>SETMETA(o,k,v)</b>: attach metadata; does not affect semantic state</li>
  </ul>

  <h3>Representative normalization</h3>
  <p>
    Relations are stored in representative-normalized form; merges rewrite incident relations so that
    semantic content is preserved while canonicalization remains stable.
  </p>
</section>
"""

time_body = """
<section class="prose">
  <h2>Time, replay, and speculation</h2>

  <h3>Replay equivalence</h3>
  <p>
    A log prefix is sufficient to reconstruct state. Incremental diffs are merely a transport optimization:
    observers may rebuild the same semantic view by applying diffs or by full replay.
  </p>

  <h3>Seekable time</h3>
  <p>
    Clients can request snapshots at arbitrary past EIDs. This is implemented by replaying a log prefix
    in an isolated kernel instance, so historical inspection cannot affect present state.
  </p>

  <h3>Speculative branches</h3>
  <p>
    Speculation is a client-local overlay: a base EID plus an overlay event sequence.
    Branches are replayed by applying the authoritative prefix, then the overlay events.
  </p>
  <p>
    Speculative events have no effect on authoritative state unless explicitly re-submitted as proposals.
  </p>
</section>
"""

views_body = """
<section class="prose">
  <h2>Views, geometry, and metadata</h2>

  <h3>Non-interference</h3>
  <p>
    Diffs and snapshots are derived views. They can be dropped, reordered, or ignored by observers
    without affecting correctness, because they are not authoritative.
  </p>

  <h3>Geometry as gauge</h3>
  <p>
    Layout hints (position, scale, clustering intent) are metadata. They are non-causal and are never
    interpreted by the kernel as semantic operations.
  </p>
  <p>
    This is the key discipline: <b>geometry is a gauge choice</b>, not meaning.
  </p>

  <h3>Functorial views (meta-theorem)</h3>
  <p>
    An admissible view is any structure-preserving mapping out of kernel state into an observer representation
    (e.g., JSON graphs, NDJSON diffs, geometric layouts), provided it cannot feed back into the kernel.
  </p>
  <p>
    Because replay is functorial over prefixes, all admissible views compose in causal order and remain coherent
    under late-join and transport differences.
  </p>
</section>
"""

collab_body = """
<section class="prose">
  <h2>Collaboration model</h2>

  <h3>The arbiter</h3>
  <p>
    Spherepop OS uses a single sequencer (“arbiter”) to assign event IDs (EIDs) and append events to the log.
    Clients submit proposals; the arbiter accepts (sequencing them) or rejects them.
  </p>

  <h3>Why one sequencer?</h3>
  <ul>
    <li>Enforces a total causal order</li>
    <li>Eliminates concurrent authoritative timelines</li>
    <li>Preserves deterministic replay as a system invariant</li>
  </ul>

  <h3>Observers</h3>
  <p>
    Observers receive diffs and/or snapshots. They may join late, rewind, or run speculative overlays.
    None of this can destabilize the authoritative log because views do not interfere with causes.
  </p>
</section>
"""

calculus_body = """
<section class="prose">
  <h2>Spherepop calculus integration</h2>
  <p>
    The Spherepop OS substrate is built to make a “calculus of semantic operations” first-class:
    equivalence, canonicalization, relation normalization, and region collapse are not application-level hacks,
    but native event types with deterministic replay.
  </p>

  <h3>Calculus primitives, OS-native</h3>
  <ul>
    <li><b>POP</b> creates handles: raw semantic atoms (no intrinsic meaning)</li>
    <li><b>MERGE</b> induces equivalence classes (canonicalization as an event)</li>
    <li><b>LINK / UNLINK</b> manipulate typed edges in a representative-normalized graph</li>
    <li><b>COLLAPSE</b> performs bulk equivalence (region-level rewriting)</li>
    <li><b>SETMETA</b> supplies non-causal structure (layout, speculative annotations, presentation gauges)</li>
  </ul>

  <h3>Operational reading</h3>
  <p>
    Think of Spherepop calculus steps as <b>authoritative semantic rewrite events</b>.
    The OS guarantees:
  </p>
  <ul>
    <li>Every rewrite is time-stamped by total order (EID)</li>
    <li>Every rewrite is replayable</li>
    <li>Derived views can render the calculus without influencing it</li>
  </ul>

  <h3>Where this goes next</h3>
  <p>
    As utilities mature, Spherepop calculus can become the unifying layer that lets “GNU-like tools”
    operate over semantic graphs (not files) while remaining deterministic under collaboration.
  </p>
</section>
"""

utilities_body = """
<section class="prose">
  <h2>Utilities roadmap (GNU-like tools, Spherepop-native)</h2>
  <p>
    You mentioned “eventually we will be making utilities in Spherepop like GNU.”
    This page sketches a sensible direction that fits the OS invariants.
  </p>

  <h3>Design constraint</h3>
  <p>
    Utilities should never mutate authoritative state through hidden side effects.
    They should emit proposed events (or local overlays), and rely on views for presentation.
  </p>

  <h3>Candidate tool families</h3>
  <ul>
    <li><b>sp</b>: a minimal CLI client (connect, propose events, tail diffs)</li>
    <li><b>sppop</b>: create objects (batch POP)</li>
    <li><b>spmerge</b>: canonicalize handles (MERGE workflows)</li>
    <li><b>splink / spunlink</b>: relation editing with representative normalization</li>
    <li><b>spgrep</b>: semantic query over relations and metadata (view-only)</li>
    <li><b>sptime</b>: seek-to-EID snapshots; time navigation tooling</li>
    <li><b>spbranch</b>: speculative overlay management (create/rebase/discard)</li>
    <li><b>spfmt</b>: view formatting (JSON graph / NDJSON diffs / text tables)</li>
  </ul>

  <h3>Safety-by-architecture</h3>
  <p>
    Tools default to non-interference: they operate as views or proposal generators.
    The arbiter remains the only committer; the log remains the only authority.
  </p>

  <p><small class="muted">This roadmap is a suggested fit to the formal invariants; refine as your actual toolchain emerges.</small></p>
</section>
"""

spec_body = """
<section class="prose">
  <h2>Formal specification</h2>
  <p>
    The canonical reference document is the formal specification PDF.
  </p>

  <h3>Included topics</h3>
  <ul>
    <li>Motivation and design goals</li>
    <li>Core ontology: objects, relations, equivalence</li>
    <li>Append-only authoritative event log and total causal order</li>
    <li>Small-step operational semantics (POP, MERGE, LINK, UNLINK, COLLAPSE, SETMETA)</li>
    <li>Replay equivalence theorem</li>
    <li>Meta-theorem: functoriality of derived views</li>
    <li>Speculation, geometry as metadata, arbiter model</li>
  </ul>

  <hr class="sep"/>

  <p>
    <a href="../Spherepop OS.pdf">Open the PDF (local link)</a>
    <br/>
    <small class="muted">If you deploy this site, host the PDF alongside the site and update the link accordingly.</small>
  </p>
</section>
"""

pages = {
    "index.html": page_template("Home", "index.html", index_body, "Spherepop OS — deterministic event-log semantic operating system."),
    "what.html": page_template("What Is Spherepop OS?", "what.html", what_body),
    "principles.html": page_template("Core Principles", "principles.html", principles_body),
    "kernel.html": page_template("Kernel Semantics", "kernel.html", kernel_body),
    "time.html": page_template("Time, Replay, Speculation", "time.html", time_body),
    "views.html": page_template("Views & Geometry", "views.html", views_body),
    "collaboration.html": page_template("Collaboration Model", "collaboration.html", collab_body),
    "calculus.html": page_template("Spherepop Calculus", "calculus.html", calculus_body),
    "utilities.html": page_template("Utilities Roadmap", "utilities.html", utilities_body),
    "spec.html": page_template("Formal Spec", "spec.html", spec_body),
    "styles.css": base_css.strip() + "\n",
}

for fn, content in pages.items():
    with open(os.path.join(base, fn), "w", encoding="utf-8") as f:
        f.write(content)

# Add a tiny README
readme = """Spherepop OS website starter (static HTML/CSS)

How to run locally:
  - Open index.html in a browser, or:
  - python -m http.server 8000
Then visit:
  http://localhost:8000/

To deploy:
  - Upload the contents of this folder to GitHub Pages / Netlify / any static host.

Note:
  - spec.html links to "../Spherepop OS.pdf" assuming you host the PDF next to the site folder.
"""
with open(os.path.join(base, "README.txt"), "w", encoding="utf-8") as f:
    f.write(readme)

# Zip it
zip_path = "/mnt/data/spherepop_site.zip"
with zipfile.ZipFile(zip_path, "w", compression=zipfile.ZIP_DEFLATED) as z:
    for root, _, files in os.walk(base):
        for file in files:
            full = os.path.join(root, file)
            rel = os.path.relpath(full, base)
            z.write(full, arcname=f"spherepop_site/{rel}")

zip_path, sorted(os.listdir(base))[:5], len(os.listdir(base))
