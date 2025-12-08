from spherepop.parser import parse
from spherepop.core import Atom, Region, Merge


def test_parse_atom():
    t = parse("a")
    assert isinstance(t, Atom)
    assert t.r.label == "a"


def test_parse_merge_chain():
    t = parse("(a b c)")
    assert isinstance(t, Merge)
