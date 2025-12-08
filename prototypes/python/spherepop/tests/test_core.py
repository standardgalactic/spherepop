from spherepop.core import Region, Atom, Pop, Merge, default_collapse, eval_term, merge


def test_basic_merge():
    s = default_collapse
    r1 = Region("a", [1])
    r2 = Region("b", [2])
    out = merge(s, r1, r2)
    assert out.label == "a+b"
    assert out.payload == [1, 2]


def test_eval_term():
    s = default_collapse
    a = Atom(Region("a", [1]))
    b = Atom(Region("b", [2]))
    t = Merge(a, b)
    out = eval_term(s, t)
    assert out.label == "a+b"
