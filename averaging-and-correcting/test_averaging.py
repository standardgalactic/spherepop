import io
import json
import random
import tempfile
import unittest
from contextlib import redirect_stderr, redirect_stdout
from datetime import datetime, timezone
from fractions import Fraction
from pathlib import Path

import averaging as av


class IdentityTests(unittest.TestCase):
    def test_decomposition_over_integers(self):
        for a in range(-40, 41):
            for b in range(-40, 41):
                for m in range(-25, 26, 5):
                    u, v = a - m, b - m
                    self.assertEqual(a * b, m * m + m * (u + v) + u * v)

    def test_decomposition_over_rationals(self):
        rng = random.Random(7)
        for _ in range(2000):
            a, b, m = (Fraction(rng.randint(-500, 500), rng.randint(1, 12)) for _ in range(3))
            u, v = a - m, b - m
            self.assertEqual(a * b, m * m + m * (u + v) + u * v)

    def test_midpoint_difference_of_squares(self):
        rng = random.Random(11)
        for _ in range(2000):
            a, b = (Fraction(rng.randint(-500, 500), rng.randint(1, 12)) for _ in range(2))
            self.assertEqual(a * b, ((a + b) / 2) ** 2 - ((a - b) / 2) ** 2)

    def test_linear_term_vanishes_only_at_midpoint(self):
        a, b = Fraction(48), Fraction(53)
        for m in [Fraction(x, 2) for x in range(60, 140)]:
            linear = m * ((a - m) + (b - m))
            self.assertEqual(linear == 0, m == (a + b) / 2)


class TraceTests(unittest.TestCase):
    def test_all_steps_verify_across_many_inputs(self):
        for a in range(-15, 16):
            for b in range(-15, 16):
                self.assertTrue(av.build_trace(a, b).ok)
                self.assertTrue(av.build_trace(a, b, 10).ok)

    def test_known_terms(self):
        t = av.build_trace(48, 52)
        self.assertEqual(t.m, 50)
        self.assertEqual(t.terms, {"m^2": 2500, "m(u+v)": 0, "uv": -4})
        t = av.build_trace(48, 53, 50)
        self.assertEqual(t.terms, {"m^2": 2500, "m(u+v)": 50, "uv": -6})
        t = av.build_trace(98, 97, 100)
        self.assertEqual(t.terms, {"m^2": 10000, "m(u+v)": -500, "uv": 6})

    def test_midpoint_adds_difference_of_squares_steps(self):
        self.assertEqual(len(av.build_trace(48, 52).steps), 6)
        self.assertEqual(len(av.build_trace(48, 53, 50).steps), 4)

    def test_half_integer_midpoint(self):
        t = av.build_trace(47, 58)
        self.assertEqual(t.m, Fraction(105, 2))
        self.assertTrue(t.ok)

    def test_render_marks_failure(self):
        t = av.build_trace(6, 7)
        t.steps[0].rhs = Fraction(999)
        self.assertFalse(t.ok)
        self.assertIn("FAIL", t.render())


class SafeEvalTests(unittest.TestCase):
    def test_arithmetic(self):
        self.assertEqual(av.safe_eval("50^2 - 2^2"), 2496)
        self.assertEqual(av.safe_eval("-(3+4)*2"), -14)
        self.assertEqual(av.safe_eval("52.5^2 - 5.5^2"), 2726)
        self.assertEqual(av.safe_eval("1/3 + 1/6"), Fraction(1, 2))

    def test_rejects_unsafe_or_unsupported(self):
        for bad in ["__import__('os')", "a + 1", "2 ** 100", "1/0", "[1]", "'x'", "(1).real", "2 ** 0.5", "True"]:
            with self.assertRaises((ValueError, SyntaxError)):
                av.safe_eval(bad)


class CheckTests(unittest.TestCase):
    def test_correct_claim(self):
        v = av.check_claim(48, 52, "50^2 - 2^2")
        self.assertTrue(v.correct)
        self.assertEqual(v.residual, 0)

    def test_forgotten_square(self):
        v = av.check_claim(48, 52, "50^2 - 2")
        self.assertEqual(v.residual, 2)
        self.assertEqual(v.claimed, 2498)
        self.assertEqual(len(v.hypotheses), 1)
        self.assertIn("uv replaced by a single factor (u or -v, = -2)", v.hypotheses[0])

    def test_single_factor_hypothesis_needs_matching_residual(self):
        v = av.check_claim(48, 52, "50^2 - 3")
        self.assertEqual(v.residual, 1)
        self.assertEqual(v.hypotheses, [])

    def test_dropped_linear_term_is_named(self):
        v = av.check_claim(48, 53, "50^2 - 2*3", m=50)
        self.assertEqual(v.residual, -50)
        self.assertTrue(any("omitted: m(u+v)" in h for h in v.hypotheses))

    def test_sign_flip_is_named(self):
        # product 98*97 = 9506; flipping the sign on uv = 6 gives 9500
        v = av.check_claim(98, 97, "100^2 - 500 - 6", m=100)
        self.assertEqual(v.residual, -12)
        self.assertTrue(any("sign flipped on: uv" in h for h in v.hypotheses))

    def test_accepted_claim_has_no_hypotheses(self):
        self.assertEqual(av.check_claim(13, 17, "15^2 - 2^2").hypotheses, [])

    def test_zero_valued_terms_are_not_blamed(self):
        v = av.check_claim(48, 52, "50^2 - 2")
        self.assertFalse(any("m(u+v)" in h for h in v.hypotheses))

    def test_log_roundtrip(self):
        v = av.check_claim(48, 53, "50^2 - 6", m=50)
        with tempfile.TemporaryDirectory() as d:
            path = Path(d) / "log.jsonl"
            fixed = datetime(2026, 10, 17, 12, 0, tzinfo=timezone.utc)
            av.log_entry(path, v, "dropped the middle term on purpose", now=fixed)
            av.log_entry(path, av.check_claim(48, 52, "50^2 - 2^2"), "symmetric about 50", now=fixed)
            rows = [json.loads(line) for line in path.read_text().splitlines()]
        self.assertEqual(len(rows), 2)
        self.assertEqual(rows[0]["justification"], "dropped the middle term on purpose")
        self.assertFalse(rows[0]["correct"])
        self.assertEqual(rows[0]["residual"], "-50")
        self.assertTrue(rows[1]["correct"])
        self.assertEqual(rows[1]["time"], "2026-10-17T12:00:00+00:00")


class ExploreTests(unittest.TestCase):
    def good(self, a, b, **kw):
        return [r.m for r in av.explore(a, b, **kw) if r.all_in_repertoire]

    def test_round_predicate(self):
        for n in (0, 5, 50, 700, 9, 90, -30):
            self.assertTrue(av.is_round(n), n)
        for n in (12, 15, 55, 101, Fraction(1, 2)):
            self.assertFalse(av.is_round(n), n)
        self.assertTrue(av.is_round(15, extra=[15]))

    def test_symmetric_pair(self):
        self.assertEqual(self.good(48, 52), [50])

    def test_two_references_for_47_times_58(self):
        rows = {r.m: r for r in av.explore(47, 58)}
        self.assertTrue(rows[Fraction(50)].all_in_repertoire)
        self.assertFalse(rows[Fraction(60)].all_in_repertoire)
        halves = {r.m: r for r in av.explore(47, 58, halves=True)}
        self.assertFalse(halves[Fraction(105, 2)].all_in_repertoire)

    def test_midpoint_not_always_in_repertoire(self):
        self.assertEqual(self.good(13, 17), [10, 20])
        self.assertEqual(self.good(13, 17, extra=[15]), [10, 15, 20])

    def test_near_a_power_of_ten(self):
        self.assertIn(Fraction(100), self.good(98, 97))

    def test_halves_flag_adds_references(self):
        self.assertEqual(len(av.explore(48, 52, halves=True)), 2 * len(av.explore(48, 52)) - 1)

    def test_no_ranking_in_output(self):
        text = av.render_rows(av.explore(48, 52), show_all=True)
        for word in ("score", "cost", "difficulty", "rank"):
            self.assertNotIn(word, text.lower())

    def test_every_listed_reference_yields_a_verified_trace(self):
        for a, b in [(48, 52), (47, 58), (13, 17), (98, 97), (48, 53)]:
            for r in av.explore(a, b):
                self.assertTrue(av.build_trace(a, b, r.m).ok)


class CommandLineTests(unittest.TestCase):
    def run_cli(self, *argv):
        out, err = io.StringIO(), io.StringIO()
        with redirect_stdout(out), redirect_stderr(err):
            code = av.main(list(argv))
        return code, out.getvalue(), err.getvalue()

    def test_trace_exit_zero(self):
        code, out, _ = self.run_cli("trace", "48", "52")
        self.assertEqual(code, 0)
        self.assertIn("all equalities verified", out)

    def test_check_exit_codes(self):
        self.assertEqual(self.run_cli("check", "48", "52", "--claim", "50^2 - 2^2")[0], 0)
        self.assertEqual(self.run_cli("check", "48", "52", "--claim", "50^2 - 2")[0], 1)

    def test_bad_expression_is_an_error_not_a_crash(self):
        code, _, err = self.run_cli("check", "48", "52", "--claim", "__import__('os')")
        self.assertEqual(code, 2)
        self.assertIn("error", err)

    def test_explore_prints_repertoire(self):
        code, out, _ = self.run_cli("explore", "47", "58")
        self.assertEqual(code, 0)
        self.assertIn("Repertoire", out)
        self.assertIn("m =     50", out)


if __name__ == "__main__":
    unittest.main()
