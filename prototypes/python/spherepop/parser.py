from __future__ import annotations
from typing import List, Union
from .core import Region, Atom, Pop, Merge, Term, default_collapse


Token = str


def tokenize(source: str) -> List[Token]:
    spaced = source.replace("(", " ( ").replace(")", " ) ")
    return [t for t in spaced.split() if t]


def parse_tokens(tokens: List[Token]) -> Term:
    if not tokens:
        raise SyntaxError("empty input")
    token = tokens.pop(0)
    if token == "(":
        items: List[Union[str, Term]] = []
        while tokens and tokens[0] != ")":
            if tokens[0] == "(":
                items.append(parse_tokens(tokens))
            else:
                items.append(tokens.pop(0))
        if not tokens:
            raise SyntaxError("missing closing )")
        tokens.pop(0)  # consume ")"
        return build_ast(items)
    elif token == ")":
        raise SyntaxError("unexpected )")
    else:
        # atom
        return Atom(Region(label=token, payload=[]))


def build_ast(items: List[Union[str, Term]]) -> Term:
    # Support simple forms:
    # (pop t)
    # (merge t1 t2)
    # ((a b c))  -> nested merges
    if not items:
        raise SyntaxError("empty list form")
    head = items[0]
    rest = items[1:]

    if isinstance(head, str) and head == "pop":
        if len(rest) != 1:
            raise SyntaxError("pop expects exactly one argument")
        t = rest[0]
        if isinstance(t, str):
            t = Atom(Region(label=t, payload=[]))
        return Pop(t)

    if isinstance(head, str) and head == "merge":
        if len(rest) != 2:
            raise SyntaxError("merge expects two arguments")
        left, right = rest
        if isinstance(left, str):
            left = Atom(Region(label=left, payload=[]))
        if isinstance(right, str):
            right = Atom(Region(label=right, payload=[]))
        return Merge(left, right)

    # Otherwise: treat as left-fold merge
    terms: List[Term] = []
    for it in items:
        if isinstance(it, str):
            terms.append(Atom(Region(label=it, payload=[])))
        else:
            terms.append(it)
    if len(terms) == 1:
        return terms[0]
    acc = terms[0]
    for t in terms[1:]:
        acc = Merge(acc, t)
    return acc


def parse(source: str) -> Term:
    tokens = tokenize(source)
    term = parse_tokens(tokens)
    if tokens:
        raise SyntaxError(f"unconsumed tokens: {tokens}")
    return term


def eval_source(source: str):
    term = parse(source)
    return default_collapse(eval_term=...)  # intentional stub, fixed in repl.py
