import unittest

from spherepop import (
    extensional_view,
    history_is_prefix,
    history_view,
    make_config,
    parse_operation,
    parse_program,
    parse_sphere,
    render_expr,
    transition,
)
from spherepop.model import BindOp, CollapseOp, PopOp, RefuseOp


class SpherepopTests(unittest.TestCase):
    def test_parse_program_and_operation_types(self):
        ops = parse_program(["POP 1", "REFUSE o2", "BIND prefix:a", "COLLAPSE B=C"])
        self.assertEqual([type(op).__name__ for op in ops], ["PopOp", "RefuseOp", "BindOp", "CollapseOp"])
        self.assertEqual(parse_operation("POP").path, None)

    def test_pop_resolves_scope_not_deletion(self):
        cfg = make_config(parse_sphere("(A (B C) D)"), {"A", "B", "C", "D"})
        out = transition(cfg, PopOp(path=(1,)))
        self.assertEqual(render_expr(out.sigma), "(A B C D)")
        self.assertEqual(out.option_space, cfg.option_space)
        self.assertEqual(len(out.history), 1)

    def test_append_only_history_prefix_monotonicity(self):
        cfg0 = make_config(parse_sphere("(A (B C) D)"), {"o1", "o2", "o3"})
        cfg1 = transition(cfg0, PopOp(path=(1,)))
        cfg2 = transition(cfg1, RefuseOp(frozenset({"o2"})))
        cfg3 = transition(cfg2, BindOp("prefix:o"))
        self.assertTrue(history_is_prefix(cfg0, cfg1))
        self.assertTrue(history_is_prefix(cfg1, cfg2))
        self.assertTrue(history_is_prefix(cfg2, cfg3))
        self.assertEqual([e.history_index for e in cfg3.history], [0, 1, 2])

    def test_refuse_is_option_space_subtraction(self):
        cfg = make_config(parse_sphere("(A B)"), {"x", "y", "z"})
        out = transition(cfg, RefuseOp(frozenset({"y", "missing"})))
        self.assertEqual(out.option_space, frozenset({"x", "z"}))
        self.assertEqual(cfg.option_space, frozenset({"x", "y", "z"}))

    def test_bind_is_predicate_filter(self):
        cfg = make_config(parse_sphere("(A B)"), {"alpha", "beta", "axis"})
        out = transition(cfg, BindOp("prefix:a"))
        self.assertEqual(out.option_space, frozenset({"alpha", "axis"}))

    def test_collapse_is_equivalence_quotient_not_destruction(self):
        cfg = make_config(parse_sphere("(A B C)"), {"A", "B", "C"})
        out = transition(cfg, CollapseOp(classes=(frozenset({"B", "C"}),)))
        self.assertEqual(render_expr(out.sigma), "(A B B)")
        self.assertEqual(out.option_space, frozenset({"A", "B"}))
        self.assertEqual(len(out.collapse_log), 1)
        self.assertEqual(out.collapse_log[0][0], 0)

    def test_distinct_histories_can_share_extensional_view(self):
        base = make_config(parse_sphere("(A (B C) D)"), {"A", "B", "C", "D"})
        h1 = transition(base, PopOp(path=(1,)))
        h2 = transition(base, BindOp("ALL"))
        h2 = transition(h2, PopOp(path=(1,)))
        self.assertNotEqual(history_view(h1), history_view(h2))
        self.assertEqual(extensional_view(h1), extensional_view(h2))

    def test_derived_views_do_not_mutate_authoritative_history(self):
        cfg = make_config(parse_sphere("(A (B C) D)"), {"o1", "o2"})
        cfg = transition(cfg, PopOp(path=(1,)))
        before = cfg.history
        _ = history_view(cfg)
        _ = extensional_view(cfg)
        self.assertEqual(cfg.history, before)


if __name__ == "__main__":
    unittest.main()
