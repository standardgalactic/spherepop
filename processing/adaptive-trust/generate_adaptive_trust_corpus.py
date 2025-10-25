import os
from textwrap import dedent

# === Output directories ===
ROOT = "Adaptive_Trust_Dynamics_Corpus"
CYCLE1_DIR = os.path.join(ROOT, "Cycle1_Diagnosis")
CYCLE2_DIR = os.path.join(ROOT, "Cycle2_Renewal")
os.makedirs(CYCLE1_DIR, exist_ok=True)
os.makedirs(CYCLE2_DIR, exist_ok=True)

# === Essay titles for each cycle ===
cycle1_titles = [
    "Entropy Budgets in Cognitive Architectures: Preventing Over-Recursion in Human–AI Symbiosis",
    "The Semantic Singularity: When Recursive Interpretation Exceeds Dissipative Capacity",
    "Curvature Contraction in Ethical Futarchies: A Field-Theoretic Approach to Moral Decision-Making",
    "Attentional Cladistics Revisited: Evolutionary Thresholds in Multi-Agent Learning Environments",
    "Sheaf-Theoretic Coherence in Distributed Cognition: Bridging RSVP and Operator Ecology",
    "The Paradox of Permeability: Trust Hysteresis in Geozotic Power Systems",
    "Bounded Immersion in Mythic Computation: Entropy Limits on Narrative Recursion",
    "Punitive Signals in Socio-Symbolic Fields: A RSVP Intervention for Governance Reform",
    "Amplitwist Operators in Neurogeometric Design: From Trust to Perceptual Stability",
    "The Termite–Neuron–Forest Triad: Marginalized Intelligences in Ecological Agency",
    "Recursive Amplification in Semantic Infrastructure: Throttling for Sustainable Scaling",
    "Field-Theoretic Patriarchy: Colonial Roots and Radical RSVP Solutions",
    "Operator Ecology in Generative Cinema: Coherence Beyond Chokepoints",
    "Yarncrawler Dynamics: Semantic Recursion in Essay Generation Pipelines",
    "Cosmological Constraints on Institutional Recursion: Deriving Entropy from RSVP Primitives",
    "Multi-Scale Temporal Dynamics in Agency Detection: From Assembly to Forest-Scale Cognition",
    "The Care–Domestication Spectrum: Thresholds in AI-Human Evolutionary Creativity",
    "Homotopy-Based Governance: Sheaf Consistency in Recursive Futarchies",
    "Entropy-Regulated Permeability in Bioforge Incubators: Trust in Biotechnological Networks",
    "The Recursive Singularity in Cultural Evolution: Bounded Narratives for Long-Term Coherence",
]

cycle2_titles = [
    "Negentropic Shocks in Institutional Renewal: Overcoming Hysteresis in Bureaucratic Systems",
    "Phase-Lock Collapse in Collaborative AI: Entropy Bounds on Multi-Model Consensus",
    "Tensorial Ethics in Recursive Markets: Curvature Metrics for Fair Futarchy",
    "Evolutionary Attention Dynamics: Cladistic Pathways in Adaptive Learning Networks",
    "Presheaf Repair in Fragmented Cognition: Operator Ecology for Mental Integration",
    "Energy–Trust Duality in Geozotic Networks: Hysteresis in Sustainable Power Sharing",
    "Entropy-Limited Myths in Digital Cultures: Bounded Recursion for Narrative Sustainability",
    "Signal Correction in Punitive Socio-Fields: RSVP for Restorative Justice",
    "Neurogeometric Phase Alignment: Amplitwist Applications in Cognitive Interfaces",
    "Fractal Agency in Ecological Triads: Entropy Coherence Across Scales",
    "Homotopy Throttles in Knowledge Amplification: Sustainable Semantic Scaling",
    "Curvature Symmetry in Social Liberation: Field-Theoretic Dismantling of Hierarchies",
    "Narrative Coherence in Media Ecologies: Operator Dynamics for Generative Stories",
    "Interpretive Temperature in Automated Narratives: Yarncrawler for Coherent Generation",
    "Institutional Cosmology: Entropy Equations for Organizational Longevity",
    "Temporal Synchronization in Multi-Agent Agency: CLIO Operators for Detection",
    "Negentropic Care in AI Evolution: Spectrum Thresholds for Mutual Adaptation",
    "Policy Alignment via Homotopy: Governance in Layered Recursions",
    "Trust Optimization in Biotech Ecosystems: SITH for Entropy-Regulated Innovation",
    "Cultural Narrative Bounds: Preventing Singularity in Evolutionary Myths",
]

# === Shared LaTeX preamble ===
preamble = dedent(r"""
\documentclass[12pt,a4paper]{article}
\usepackage[utf8]{inputenc}
\usepackage[T1]{fontenc}
\usepackage{lmodern}
\usepackage{microtype}
\usepackage{amsmath,amssymb,amsthm}
\usepackage{mathtools}
\usepackage{geometry}
\usepackage{bm}
\usepackage{hyperref}
\usepackage{titlesec}
\usepackage{enumitem}
\usepackage{parskip}
\geometry{margin=1in}
\title{%TITLE%}
\author{Flyxion}
\date{October 2025}
\begin{document}
\maketitle
\textbf{Abstract:} %ABSTRACT%

\bigskip
\textbf{Outline:}
\begin{enumerate}
\item %OUTLINE%
\end{enumerate}

\bigskip
\textbf{Cross-reference:} %CROSSREF%
\end{document}
""").strip()

# === Simple placeholders for abstract, outline, and cross-reference ===
def make_essay_file(cycle, idx, title):
    label = f"{cycle}.{idx}"
    filename = f"essay_{cycle}_{idx:02d}.tex"
    outdir = CYCLE1_DIR if cycle == 1 else CYCLE2_DIR
    outpath = os.path.join(outdir, filename)
    paired = f"[{2 if cycle==1 else 1}.{idx}]"
    cross = f"See counterpart essay {paired} for the dual perspective."
    content = (
        preamble.replace("%TITLE%", title)
        .replace("%ABSTRACT%", "To be filled.")
        .replace("%OUTLINE%", "To be outlined.")
        .replace("%CROSSREF%", cross)
    )
    with open(outpath, "w", encoding="utf-8") as f:
        f.write(content)
    print(f"✅ Created {outpath}")

# === Generate all 40 essays ===
for i, t in enumerate(cycle1_titles, 1):
    make_essay_file(1, i, t)
for i, t in enumerate(cycle2_titles, 1):
    make_essay_file(2, i, t)

# === Create a master index ===
index_path = os.path.join(ROOT, "index.tex")
with open(index_path, "w", encoding="utf-8") as f:
    f.write(dedent(r"""
    \documentclass[12pt,a4paper]{article}
    \usepackage[utf8]{inputenc}
    \usepackage[T1]{fontenc}
    \usepackage{hyperref}
    \usepackage{geometry}
    \geometry{margin=1in}
    \title{\Huge Adaptive Trust Dynamics Corpus}
    \author{Flyxion}
    \date{October 2025}
    \begin{document}
    \maketitle
    \tableofcontents
    \newpage
    """).strip() + "\n")

    for i in range(1, 21):
        f.write(f"\\input{{Cycle1_Diagnosis/essay_1_{i:02d}.tex}}\n\\newpage\n")
    for i in range(1, 21):
        f.write(f"\\input{{Cycle2_Renewal/essay_2_{i:02d}.tex}}\n\\newpage\n")

    f.write("\\end{document}\n")

print(f"\nAll essays and master index created under: {ROOT}")

