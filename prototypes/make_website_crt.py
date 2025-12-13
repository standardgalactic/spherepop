# Create full Spherepop OS CRT website and zip it

import os, zipfile, datetime, textwrap, pathlib

base = "/mnt/data/spherepop_crt_site"
os.makedirs(base, exist_ok=True)

def write(name, content):
    with open(os.path.join(base, name), "w", encoding="utf-8") as f:
        f.write(content.strip() + "\n")

# ========== styles.css (base + CRT) ==========
write("styles.css", """
/* Spherepop OS — Base + CRT Variant */

:root{
  --bg:#060906;
  --panel:#0b120d;
  --text:#caffdf;
  --muted:#9fcbb2;
  --accent:#53ff9a;
  --border:#1c2b22;
  --mono: ui-monospace, Menlo, Consolas, monospace;
  --sans: system-ui, -apple-system, Segoe UI, Roboto, Helvetica, Arial;
}

*{box-sizing:border-box}
body{
  margin:0;
  font-family:var(--sans);
  background:
    repeating-linear-gradient(
      to bottom,
      rgba(0,0,0,0.00),
      rgba(0,0,0,0.00) 2px,
      rgba(0,0,0,0.06) 3px
    ),
    radial-gradient(1200px 700px at 20% 10%, rgba(83,255,154,.12), transparent 60%),
    #060906;
  color:var(--text);
  text-shadow:0 0 6px rgba(83,255,154,.18);
  line-height:1.55;
}

header{
  position:sticky; top:0;
  background:rgba(6,9,6,.85);
  border-bottom:1px solid var(--border);
}

.container{max-width:1100px; margin:0 auto; padding:16px}

nav a{
  color:var(--text);
  margin-right:12px;
  text-decoration:none;
  font-size:13px;
}
nav a:hover{color:var(--accent)}

.hero, .prose, .card{
  border:1px solid rgba(83,255,154,.35);
  background:rgba(6,9,6,.82);
  padding:22px;
  border-radius:14px;
  margin-bottom:22px;
}

h1,h2,h3{color:var(--accent)}
p,li{color:var(--muted)}

.codeblock{
  font-family:var(--mono);
  background:#020402;
  padding:14px;
  border-radius:10px;
  border:1px solid rgba(83,255,154,.4);
  color:#b9ffd9;
  margin:14px 0;
}

footer{
  border-top:1px solid var(--border);
  font-size:12px;
  color:var(--muted);
  padding:16px;
}
""")

# ========== Common header/footer template ==========
def page(title, body):
    nav = """
<nav>
<a href="index.html">Home</a>
<a href="what.html">What</a>
<a href="principles.html">Principles</a>
<a href="kernel.html">Kernel</a>
<a href="views.html">Views</a>
<a href="calculus.html">Calculus</a>
<a href="utilities.html">Utilities</a>
<a href="spec.html">Spec</a>
</nav>
"""
    return f"""
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8"/>
<title>{title} — Spherepop OS</title>
<link rel="stylesheet" href="styles.css"/>
</head>
<body>
<header class="container">
<h1>Spherepop OS</h1>
{nav}
</header>
<main class="container">
{body}
</main>
<footer class="container">
Spherepop OS · Flyxion · {datetime.date.today().year}
</footer>
</body>
</html>
"""

# ========== Pages ==========

write("index.html", page("Home", """
<section class="hero">
<h2>Deterministic Semantic Operating System</h2>
<p>
Spherepop OS is an operating system whose sole authority is an append-only,
totally ordered event log. All state is derived by deterministic replay.
</p>
</section>
"""))

write("what.html", page("What Is Spherepop OS", """
<section class="prose">
<h2>What is Spherepop OS?</h2>
<p>
Spherepop OS replaces mutable global state with time-ordered semantic events.
Objects, relations, and equivalence exist only as consequences of replay.
</p>
</section>
"""))

write("principles.html", page("Core Principles", """
<section class="prose">
<ul>
<li>Append-only authority</li>
<li>Total causal order</li>
<li>Deterministic replay</li>
<li>View–cause separation</li>
<li>Semantic primacy</li>
</ul>
</section>
"""))

write("kernel.html", page("Kernel Semantics", """
<section class="prose">
<p>The kernel is a deterministic transition system:</p>
<div class="codeblock">
σ = (O, U, R, M)
σ —e→ σ′
</div>
<p>
Kernel transitions are pure functions of prior state and next event.
</p>
</section>
"""))

write("views.html", page("Views & Functoriality", """
<section class="prose">
<p>
Views are functors out of kernel state. Geometry is metadata; layout is gauge.
</p>
</section>
"""))

write("calculus.html", page("Spherepop Calculus", """
<section class="prose">
<p>
Spherepop calculus is the algebra of authoritative events:
POP, MERGE, LINK, UNLINK, COLLAPSE, SETMETA.
</p>
</section>
"""))

write("utilities.html", page("Utilities Roadmap", """
<section class="prose">
<p>
Spherepop utilities resemble GNU tools but operate over semantic events,
relations, and views rather than files.
</p>
</section>
"""))

write("spec.html", page("Formal Specification", """
<section class="prose">
<p>
See the formal PDF specification for full operational semantics.
</p>
</section>
"""))

# ========== README ==========
write("README.txt", """
Spherepop OS CRT Website

Static HTML/CSS.
Host anywhere (GitHub Pages, Netlify).

Enable CRT feel via default styles.css.
""")

# ========== ZIP ==========
zip_path = "/mnt/data/spherepop_crt_site.zip"
with zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED) as z:
    for file in os.listdir(base):
        z.write(os.path.join(base, file), arcname=file)

zip_path