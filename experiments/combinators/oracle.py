"""Independent reference for the S/K experiment.

Terms are plain Python values: an atom is a string, an application is a 2-tuple
(function, argument). There is no interning and no cached redex flag, so the code
shares no mechanism with cl.mjs. Requests arrive on stdin as a JSON list and answers
leave on stdout as a JSON list.

Wire format: atoms are strings, applications are 2-element lists, lambda terms are
a string (variable), ["@", f, x] (application) or ["\\", name, body] (abstraction).
"""
import json
import sys
import threading

I_TERM = (("S", "K"), "K")


def to_tuple(w):
    return w if isinstance(w, str) else (to_tuple(w[0]), to_tuple(w[1]))


def to_wire(t):
    return t if isinstance(t, str) else [to_wire(t[0]), to_wire(t[1])]


def is_k(t):
    return isinstance(t, tuple) and isinstance(t[0], tuple) and t[0][0] == "K"


def is_s(t):
    return (
        isinstance(t, tuple)
        and isinstance(t[0], tuple)
        and isinstance(t[0][0], tuple)
        and t[0][0][0] == "S"
    )


def is_redex(t):
    return is_k(t) or is_s(t)


def contract(t):
    if is_k(t):
        return t[0][1]
    x, y, z = t[0][0][1], t[0][1], t[1]
    return ((x, z), (y, z))


def step_lo(t):
    if is_redex(t):
        return contract(t)
    if isinstance(t, tuple):
        r = step_lo(t[0])
        if r is not None:
            return (r, t[1])
        r = step_lo(t[1])
        if r is not None:
            return (t[0], r)
    return None


def step_li(t):
    if isinstance(t, tuple):
        r = step_li(t[0])
        if r is not None:
            return (r, t[1])
        r = step_li(t[1])
        if r is not None:
            return (t[0], r)
        if is_redex(t):
            return contract(t)
    return None


def step_ri(t):
    if isinstance(t, tuple):
        r = step_ri(t[1])
        if r is not None:
            return (t[0], r)
        r = step_ri(t[0])
        if r is not None:
            return (r, t[1])
        if is_redex(t):
            return contract(t)
    return None


def step_ro(t):
    if is_redex(t):
        return contract(t)
    if isinstance(t, tuple):
        r = step_ro(t[1])
        if r is not None:
            return (t[0], r)
        r = step_ro(t[0])
        if r is not None:
            return (r, t[1])
    return None


STEPPERS = {"lo": step_lo, "li": step_li, "ri": step_ri, "ro": step_ro}


def size(t):
    return 0 if isinstance(t, str) else 1 + size(t[0]) + size(t[1])


def normalize(t, strategy, max_steps, max_size):
    f = STEPPERS[strategy]
    seen = {t: 0}
    steps = 0
    while True:
        n = f(t)
        if n is None:
            return {"status": "normal", "steps": steps, "term": to_wire(t)}
        t = n
        steps += 1
        if t in seen:
            return {"status": "cycle", "steps": steps, "term": to_wire(t), "period": steps - seen[t]}
        seen[t] = steps
        if steps >= max_steps:
            return {"status": "budget", "steps": steps, "term": to_wire(t)}
        if size(t) > max_size:
            return {"status": "size", "steps": steps, "term": to_wire(t)}


def gk(t):
    if is_k(t):
        return gk(t[0][1])
    if is_s(t):
        x, y, z = gk(t[0][0][1]), gk(t[0][1]), gk(t[1])
        return ((x, z), (y, z))
    if isinstance(t, tuple):
        return (gk(t[0]), gk(t[1]))
    return t


def has_var(t, name):
    if isinstance(t, str):
        return t == name and name not in ("S", "K")
    return has_var(t[0], name) or has_var(t[1], name)


def abstract(name, m, eta=False):
    if not has_var(m, name):
        return ("K", m)
    if isinstance(m, str):
        return I_TERM
    if eta and isinstance(m[1], str) and m[1] == name and not has_var(m[0], name):
        return m[0]
    return (("S", abstract(name, m[0], eta)), abstract(name, m[1], eta))


def abstraction_steps(name, m):
    if not has_var(m, name):
        return 1
    if isinstance(m, str):
        return 2
    return 1 + abstraction_steps(name, m[0]) + abstraction_steps(name, m[1])


def lam_to_cl(a, eta):
    if isinstance(a, str):
        return a
    if a[0] == "@":
        return (lam_to_cl(a[1], eta), lam_to_cl(a[2], eta))
    return abstract(a[1], lam_to_cl(a[2], eta), eta)


def run_machine(prog, regs, max_steps):
    regs = list(regs)
    pc = 0
    steps = 0
    while steps < max_steps:
        if pc >= len(prog) or prog[pc]["op"] == "halt":
            return {"halted": True, "steps": steps, "regs": regs}
        ins = prog[pc]
        if ins["op"] == "inc":
            regs[ins["r"]] += 1
            pc = ins["next"]
        elif regs[ins["r"]] > 0:
            regs[ins["r"]] -= 1
            pc = ins["nz"]
        else:
            pc = ins["z"]
        steps += 1
    return {"halted": False, "steps": steps, "regs": regs}


def handle(req):
    op = req["op"]
    if op == "normalize":
        return normalize(to_tuple(req["term"]), req["strategy"], req["maxSteps"], req.get("maxSize", 10**9))
    if op == "gk":
        return to_wire(gk(to_tuple(req["term"])))
    if op == "abstract":
        m = to_tuple(req["term"])
        return {
            "term": to_wire(abstract(req["name"], m, req.get("eta", False))),
            "steps": abstraction_steps(req["name"], m),
        }
    if op == "tocl":
        return to_wire(lam_to_cl(req["ast"], req.get("eta", False)))
    if op == "machine":
        return run_machine(req["prog"], req["regs"], req["maxSteps"])
    raise ValueError("unknown op " + op)


def main():
    out = [handle(r) for r in json.load(sys.stdin)]
    sys.stdout.write(json.dumps(out))


if __name__ == "__main__":
    sys.setrecursionlimit(1_000_000)
    threading.stack_size(512 * 1024 * 1024)
    th = threading.Thread(target=main)
    th.start()
    th.join()
