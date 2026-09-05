import unittest

from spherepop import (
    EvalError,
    ParseError,
    extensional_view,
    history_is_prefix,
    history_view,
    make_config,
    parse_operation,
    parse_program,
    parse_sphere,
    render_expr,
    representative,
    transition,
)
from spherepop.model import BindOp, CollapseOp, PopOp, Quotient, RefuseOp


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

    def test_pop_by_label_resolves_to_same_result_as_path(self):
        from spherepop.grammar import parse_sphere as grammar_parse_sphere

        sigma = grammar_parse_sphere("(outer: A (inner: B C) D)")
        cfg = make_config(sigma, {"x"})
        by_label = transition(cfg, PopOp(label="inner"))
        by_path = transition(cfg, PopOp(path=(1,)))
        self.assertEqual(render_expr(by_label.sigma), render_expr(by_path.sigma))
        self.assertEqual(by_label.history[-1].label, "inner")
        self.assertIsNone(by_path.history[-1].label)

    def test_pop_rejects_both_path_and_label(self):
        cfg = make_config(parse_sphere("(A (B C) D)"), {"x"})
        with self.assertRaises(EvalError):
            transition(cfg, PopOp(path=(1,), label="inner"))

    def test_pop_by_label_not_found(self):
        cfg = make_config(parse_sphere("(A (B C) D)"), {"x"})
        with self.assertRaises(EvalError):
            transition(cfg, PopOp(label="nonexistent"))

    def test_pop_by_label_ambiguous(self):
        from spherepop.grammar import parse_sphere as grammar_parse_sphere

        # Two sibling spheres sharing a label -- resolution must refuse to
        # guess which one "inner" means.
        sigma = grammar_parse_sphere("(outer: (inner: A) (inner: B))")
        cfg = make_config(sigma, {"x"})
        with self.assertRaises(EvalError):
            transition(cfg, PopOp(label="inner"))

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

    def test_refuse_requires_nonempty_target_appendix_e(self):
        cfg = make_config(parse_sphere("(A B)"), {"x", "y", "z"})
        # Every requested name is absent from the option space, so nothing
        # is actually being refused -- Appendix E requires R nonempty.
        with self.assertRaises(EvalError):
            transition(cfg, RefuseOp(frozenset({"missing", "also-missing"})))
        with self.assertRaises(ParseError):
            parse_operation("REFUSE")

    def test_bind_is_predicate_filter(self):
        cfg = make_config(parse_sphere("(A B)"), {"alpha", "beta", "axis"})
        out = transition(cfg, BindOp("prefix:a"))
        self.assertEqual(out.option_space, frozenset({"alpha", "axis"}))

    def test_collapse_is_equivalence_quotient_not_destruction(self):
        cfg = make_config(parse_sphere("(A B C)"), {"A", "B", "C"})
        out = transition(cfg, CollapseOp(classes=(frozenset({"B", "C"}),)))

        # Display renders a chosen representative ("B", sorted-first)...
        self.assertEqual(render_expr(out.sigma), "(A B B)")

        # ...but the option space holds the class itself, not that string.
        # "B" and "C" individually are no longer present at all: only the
        # Quotient is, so no downstream code can privilege the
        # representative by comparing against "B" directly.
        quotient = Quotient(members=frozenset({"B", "C"}))
        self.assertEqual(out.option_space, frozenset({"A", quotient}))
        self.assertNotIn("B", out.option_space)
        self.assertNotIn("C", out.option_space)

        self.assertEqual(len(out.collapse_log), 1)
        self.assertEqual(out.collapse_log[0][0], 0)

    def test_quotient_equality_ignores_construction_order(self):
        # Two Quotients built from the same members are equal regardless
        # of the order the members were supplied in -- there is no
        # representative field to disagree about.
        self.assertEqual(Quotient(frozenset({"B", "C"})), Quotient(frozenset({"C", "B"})))
        self.assertEqual(representative(Quotient(frozenset({"B", "C"}))), "B")

    def test_extensional_view_sorts_mixed_string_and_quotient_options(self):
        # A raw sorted(option_space) breaks once it can hold a mix of
        # strings and Quotients (not mutually orderable); extensional_view
        # must not have that problem.
        cfg = make_config(parse_sphere("(A B C)"), {"A", "B", "C"})
        out = transition(cfg, CollapseOp(classes=(frozenset({"B", "C"}),)))
        self.assertEqual(extensional_view(out), ("(A B B)", ("A", "B")))

    def test_bind_after_collapse_is_existential_over_class_members(self):
        cfg = make_config(parse_sphere("(A B C)"), {"alpha", "beta", "gamma"})
        collapsed = transition(cfg, CollapseOp(classes=(frozenset({"alpha", "gamma"}),)))
        # "beta" stays a plain string; {"alpha","gamma"} becomes one Quotient.
        out = transition(collapsed, BindOp("prefix:a"))
        # The class is admitted because "alpha" (one of its members)
        # matches, even though "gamma" does not -- the provisional
        # existential reading documented in semantics._predicate.
        self.assertEqual(out.option_space, frozenset({Quotient(frozenset({"alpha", "gamma"}))}))

    def test_refuse_after_collapse_removes_whole_class(self):
        cfg = make_config(parse_sphere("(A B C)"), {"alpha", "beta", "gamma"})
        collapsed = transition(cfg, CollapseOp(classes=(frozenset({"alpha", "gamma"}),)))
        out = transition(collapsed, RefuseOp(frozenset({"alpha"})))
        # Naming one member of the class refuses the whole class.
        self.assertEqual(out.option_space, frozenset({"beta"}))
        # The event records only what was actually named, not the
        # collateral member swept out with it.
        self.assertEqual(out.history[-1].refused, frozenset({"alpha"}))

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
