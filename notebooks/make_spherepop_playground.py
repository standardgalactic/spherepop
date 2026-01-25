import nbformat as nbf

# Create notebook with explicit metadata
nb = nbf.v4.new_notebook(
    metadata={
        "kernelspec": {
            "display_name": "Python 3",
            "language": "python",
            "name": "python3"
        },
        "language_info": {
            "name": "python",
            "version": "3.x"
        }
    }
)

cells = []

def md(text):
    cells.append(nbf.v4.new_markdown_cell(text))

def code(text):
    cells.append(nbf.v4.new_code_cell(text))

# --------------------
# Notebook content
# --------------------

md("""
# Spherepop Playground Notebook

A minimal executable toy model of **Spherepop**.

**Core ideas**
- Spheres = delayed irreversible events  
- `pop()` = evaluates once and records history  
- Identity = event provenance  
- Nested scopes = compositional structure  
- Geometry emerges from causal structure
""")

md("## Imports and Core Structures")
code("""
import uuid
from dataclasses import dataclass, field
from typing import Callable, Any, List, Tuple, Dict
import matplotlib.pyplot as plt
""")

md("## Event, History, Sphere")
code("""
@dataclass
class Event:
    eid: str
    label: str
    value: Any
    parents: Tuple[str, ...]

@dataclass
class History:
    events: Dict[str, Event] = field(default_factory=dict)

    def add(self, event: Event):
        self.events[event.eid] = event

    def lineage(self, eid: str) -> List[str]:
        seen = set()
        order = []
        def dfs(x):
            if x in seen:
                return
            seen.add(x)
            for p in self.events[x].parents:
                dfs(p)
            order.append(x)
        dfs(eid)
        return order

@dataclass
class Sphere:
    label: str
    thunk: Callable[['History'], Any]
    _eid: str = None

    def pop(self, history: History):
        if self._eid is not None:
            return history.events[self._eid].value, self._eid

        value, parents = self.thunk(history)
        eid = str(uuid.uuid4())
        history.add(Event(eid, self.label, value, parents))
        self._eid = eid
        return value, eid
""")

md("## Primitive Constructors")
code("""
def const(label, x):
    return Sphere(label, lambda h: (x, ()))

def add(label, a, b):
    def thunk(h):
        va, ea = a.pop(h)
        vb, eb = b.pop(h)
        return va + vb, (ea, eb)
    return Sphere(label, thunk)

def mul(label, a, b):
    def thunk(h):
        va, ea = a.pop(h)
        vb, eb = b.pop(h)
        return va * vb, (ea, eb)
    return Sphere(label, thunk)
""")

md("## Example: (1 + 2) × 3")
code("""
H = History()

s1 = const("one", 1)
s2 = const("two", 2)
s3 = const("three", 3)

expr = mul("mul", add("add", s1, s2), s3)
expr.pop(H)
""")

md("## Identity by History")
code("""
H2 = History()
expr2 = mul("mul", add("add", const("one",1), const("two",2)), const("three",3))
expr2.pop(H2)

len(H.events), len(H2.events)
""")

md("## Event-History Geometry")
code("""
def depth(h, eid):
    e = h.events[eid]
    if not e.parents:
        return 0
    return 1 + max(depth(h, p) for p in e.parents)

nodes = list(H.events)
depths = {eid: depth(H, eid) for eid in nodes}

layers = {}
for eid, d in depths.items():
    layers.setdefault(d, []).append(eid)

pos = {}
for d, eids in layers.items():
    for i, eid in enumerate(eids):
        pos[eid] = (i, -d)

plt.figure()
for eid, e in H.events.items():
    x, y = pos[eid]
    plt.scatter([x], [y])
    plt.text(x + 0.02, y + 0.02, e.label)
    for p in e.parents:
        xp, yp = pos[p]
        plt.plot([xp, x], [yp, y])

plt.axis("off")
plt.title("Spherepop Event-History Geometry")
plt.show()
""")

# Attach cells
nb["cells"] = cells

# Write file
path = "Spherepop_Playground.ipynb"
with open(path, "w", encoding="utf-8") as f:
    nbf.write(nb, f)

print(f"Wrote {path}")

