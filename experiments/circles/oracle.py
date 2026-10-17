"""JSON bridge to the book's actual Python interpreter, not a second oracle."""
import importlib.util
import json
from pathlib import Path
import sys

SOURCE = Path(__file__).resolve().parents[2] / 'spherepop-book-source' / 'spherepop.py'
spec = importlib.util.spec_from_file_location('spherepop_book_reference', SOURCE)
book = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = book
spec.loader.exec_module(book)
if hasattr(sys, 'set_int_max_str_digits'):
    sys.set_int_max_str_digits(0)


def decode(term):
    kind = term['kind']
    if kind == 'num':
        return int(term['value'])
    if kind == 'sym':
        return book.Sym(term['name'])
    if kind == 'color':
        return book.Color(term['name'])
    if kind == 'image':
        return book.Image(tuple(tuple(row) for row in term['rows']))
    return book.Circle(term['op'], tuple(decode(a) for a in term['args']))


def encode(term):
    if isinstance(term, int):
        return {'kind': 'num', 'value': str(term)}
    if isinstance(term, book.Sym):
        return {'kind': 'sym', 'name': term.name}
    if isinstance(term, book.Color):
        return {'kind': 'color', 'name': term.name}
    if isinstance(term, book.Image):
        return {'kind': 'image', 'rows': [list(row) for row in term.rows]}
    return {'kind': 'circle', 'op': term.op, 'args': [encode(a) for a in term.args]}


def run(case):
    term = decode(case['term'])
    if 'env' in case:
        term = book.substitute(term, {k: decode(v) for k, v in case['env'].items()})
    result, pops = book.normalize(term)
    step = book.step(term)
    return {'term': encode(result), 'pops': pops, 'size': book.size(term),
            'step': None if step is None else encode(step)}


if __name__ == '__main__':
    print(json.dumps([run(case) for case in json.load(sys.stdin)]))
