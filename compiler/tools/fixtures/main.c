/* main.c — Flat fixture conformance runner (C).
 *
 * Loads every *.json fixture from experiments/flat/fixtures/ (path
 * overridable via the first CLI argument), replays its event sequence
 * through the canonical Arbiter (kernel.c/collapse_rules.c/sugar.c), and
 * checks the resulting state against the fixture's `expect` block. This
 * is the C-side counterpart to spherepop-kernel/src/bin/fixtures.rs
 * (Rust) and experiments/flat/run_python.py (Python) -- three
 * independent implementations of the same canonical model, checked
 * against the exact same fixture files, per the tracking issue's
 * "Portable" / cross-implementation conformance criterion.
 *
 * Usage:
 *   sp_fixtures [path/to/fixtures/dir]
 */
#define _POSIX_C_SOURCE 200809L
#include <dirent.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "collapse_rules.h"
#include "json.h"
#include "kernel.h"
#include "sugar.h"

#ifndef FIXTURES_DEFAULT_DIR
#define FIXTURES_DEFAULT_DIR "../experiments/flat/fixtures"
#endif

/* ---------------- Failure message list ---------------- */

typedef struct { char **items; size_t len, cap; } FailureList;

static void flist_init(FailureList *f) { f->items = NULL; f->len = 0; f->cap = 0; }
static void flist_free(FailureList *f) {
    for (size_t i = 0; i < f->len; i++) free(f->items[i]);
    free(f->items);
}
static void flist_addf(FailureList *f, const char *fmt, ...) {
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    if (f->len >= f->cap) { f->cap = f->cap ? f->cap * 2 : 4; f->items = realloc(f->items, f->cap * sizeof(char *)); }
    f->items[f->len++] = strdup(buf);
}

/* ---------------- op -> Event(s) dispatch ---------------- */

/* Returns number of events written to `out` (capacity `cap`, >= 2), or
 * (size_t)-1 if the op JSON was malformed (missing a required field). */
static size_t events_from_op(const Json *ev, Event *out, size_t cap) {
    const char *op = json_get_str(ev, "op", NULL);
    if (!op) return (size_t)-1;
    uint64_t a, b, taken, rejected, object, key;

    if (strcmp(op, "pop") == 0) {
        if (!json_as_u64(json_get(ev, "a"), &a)) return (size_t)-1;
        out[0] = event_pop(a);
        return 1;
    }
    if (strcmp(op, "refuse") == 0) {
        if (!json_as_u64(json_get(ev, "a"), &a)) return (size_t)-1;
        out[0] = event_refuse(a, json_get_str(ev, "reason", ""));
        return 1;
    }
    if (strcmp(op, "refuse_bind") == 0) {
        if (!json_as_u64(json_get(ev, "a"), &a)) return (size_t)-1;
        if (!json_as_u64(json_get(ev, "b"), &b)) return (size_t)-1;
        out[0] = event_refuse_bind(a, b, json_get_str(ev, "reason", ""));
        return 1;
    }
    if (strcmp(op, "bind") == 0) {
        if (!json_as_u64(json_get(ev, "a"), &a)) return (size_t)-1;
        if (!json_as_u64(json_get(ev, "b"), &b)) return (size_t)-1;
        out[0] = event_bind(a, b, json_get_str(ev, "tag", ""));
        return 1;
    }
    if (strcmp(op, "collapse") == 0) {
        const char *rule = json_get_str(ev, "rule", NULL);
        if (!rule) return (size_t)-1;
        out[0] = event_collapse(rule);
        return 1;
    }
    if (strcmp(op, "link") == 0) {
        if (!json_as_u64(json_get(ev, "a"), &a)) return (size_t)-1;
        if (!json_as_u64(json_get(ev, "b"), &b)) return (size_t)-1;
        return sugar_link(a, b, json_get_str(ev, "tag", ""), out, cap);
    }
    if (strcmp(op, "unlink") == 0) {
        if (!json_as_u64(json_get(ev, "a"), &a)) return (size_t)-1;
        if (!json_as_u64(json_get(ev, "b"), &b)) return (size_t)-1;
        return sugar_unlink(a, b, out, cap);
    }
    if (strcmp(op, "choice") == 0) {
        if (!json_as_u64(json_get(ev, "taken"), &taken)) return (size_t)-1;
        if (!json_as_u64(json_get(ev, "rejected"), &rejected)) return (size_t)-1;
        return sugar_choice(taken, rejected, out, cap);
    }
    if (strcmp(op, "merge") == 0) {
        if (!json_as_u64(json_get(ev, "a"), &a)) return (size_t)-1;
        if (!json_as_u64(json_get(ev, "b"), &b)) return (size_t)-1;
        const char *rule = json_get_str(ev, "rule", NULL);
        if (!rule) return (size_t)-1;
        return sugar_merge(a, b, rule, out, cap);
    }
    if (strcmp(op, "set_meta") == 0) {
        if (!json_as_u64(json_get(ev, "object"), &object)) return (size_t)-1;
        if (!json_as_u64(json_get(ev, "key"), &key)) return (size_t)-1;
        return sugar_set_meta(object, key, out, cap);
    }
    return (size_t)-1;
}

/* ---------------- fixture execution ---------------- */

typedef enum { OUTCOME_PASS, OUTCOME_MANUAL_SKIP, OUTCOME_FAIL } Outcome;

/* Builds and submits one sub-history (`history_a` / `history_b` of the Meld
 * fixture) through its own Arbiter. Returns true on success (with `arb`
 * initialized and owned by the caller, who must call arbiter_free); on
 * failure records a failure message and leaves `arb` uninitialized. */
static bool run_sub_history(const Json *sub, Arbiter *arb, FailureList *failures) {
    const Json *omega_arr = json_get(sub, "initial_option_space");
    size_t n_omega = json_array_len(omega_arr);
    ObjectId *omega0 = malloc((n_omega ? n_omega : 1) * sizeof(ObjectId));
    for (size_t i = 0; i < n_omega; i++) json_as_u64(json_array_get(omega_arr, i), &omega0[i]);

    const Json *rules_arr = json_get(sub, "certified_rules");
    size_t n_rules = json_array_len(rules_arr);
    const char **rules = malloc((n_rules ? n_rules : 1) * sizeof(char *));
    for (size_t i = 0; i < n_rules; i++) rules[i] = json_as_str(json_array_get(rules_arr, i));

    arbiter_init(arb, omega0, n_omega, rules, n_rules);
    free(omega0);
    free(rules);

    const Json *events_arr = json_get(sub, "events");
    size_t n_events = json_array_len(events_arr);
    for (size_t i = 0; i < n_events; i++) {
        const Json *ev = json_array_get(events_arr, i);
        Event batch[4];
        size_t n = events_from_op(ev, batch, 4);
        if (n == (size_t)-1) {
            flist_addf(failures, "malformed sub-history event at index %zu", i);
            arbiter_free(arb);
            return false;
        }
        char err[128] = {0};
        if (!arbiter_submit(arb, batch, n, err)) {
            flist_addf(failures, "sub-history event at index %zu was rejected: %s", i, err);
            for (size_t k = 0; k < n; k++) event_free(&batch[k]);
            arbiter_free(arb);
            return false;
        }
    }
    return true;
}

/* Executes a two-history Meld fixture (`history_a`/`history_b`) end-to-end:
 * each sub-history runs through its own Arbiter, then the two resulting
 * histories are melded (event-log concatenation) and checked against
 * `expect_melded_history_len`. */
static Outcome run_meld_fixture(const Json *fixture, FailureList *failures) {
    Arbiter arb_a, arb_b;
    if (!run_sub_history(json_get(fixture, "history_a"), &arb_a, failures)) return OUTCOME_FAIL;
    if (!run_sub_history(json_get(fixture, "history_b"), &arb_b, failures)) {
        arbiter_free(&arb_a);
        return OUTCOME_FAIL;
    }

    /* Meld: parallel composition of two independently-generated histories
     * (the free monoidal tensor) -- concatenation of event logs, mirroring
     * spherepop-kernel::History::meld (spherepop-kernel/src/history.rs). */
    size_t melded_len = arbiter_len(&arb_a) + arbiter_len(&arb_b);

    uint64_t expected;
    if (json_as_u64(json_get(fixture, "expect_melded_history_len"), &expected)) {
        if (melded_len != expected) {
            flist_addf(failures, "expect_melded_history_len: expected %llu, got %zu",
                       (unsigned long long)expected, melded_len);
        }
    }

    arbiter_free(&arb_a);
    arbiter_free(&arb_b);
    return failures->len > 0 ? OUTCOME_FAIL : OUTCOME_PASS;
}

static Outcome run_executable_fixture(const Json *fixture, FailureList *failures) {
    const Json *omega_arr = json_get(fixture, "initial_option_space");
    size_t n_omega = json_array_len(omega_arr);
    ObjectId *omega0 = malloc((n_omega ? n_omega : 1) * sizeof(ObjectId));
    for (size_t i = 0; i < n_omega; i++) json_as_u64(json_array_get(omega_arr, i), &omega0[i]);

    const Json *rules_arr = json_get(fixture, "certified_rules");
    size_t n_rules = json_array_len(rules_arr);
    const char **rules = malloc((n_rules ? n_rules : 1) * sizeof(char *));
    for (size_t i = 0; i < n_rules; i++) rules[i] = json_as_str(json_array_get(rules_arr, i));

    Arbiter arb;
    arbiter_init(&arb, omega0, n_omega, rules, n_rules);
    free(omega0);
    free(rules);

    const Json *events_arr = json_get(fixture, "events");
    size_t n_events = json_array_len(events_arr);
    for (size_t i = 0; i < n_events; i++) {
        const Json *ev = json_array_get(events_arr, i);
        Event batch[4];
        size_t n = events_from_op(ev, batch, 4);
        if (n == (size_t)-1) {
            flist_addf(failures, "malformed fixture event at index %zu", i);
            continue;
        }
        bool expect_reject = json_get_bool(ev, "expect_reject", false);
        size_t len_before = arbiter_len(&arb);
        char err[128] = {0};
        bool accepted = arbiter_submit(&arb, batch, n, err);

        if (expect_reject) {
            if (accepted) {
                flist_addf(failures, "event at index %zu was accepted but fixture expected rejection", i);
            } else {
                if (arbiter_len(&arb) != len_before) {
                    flist_addf(failures, "rejected event mutated history length");
                }
                const char *expected_err = json_get_str(ev, "expect_error", NULL);
                if (expected_err && strncmp(err, expected_err, strlen(expected_err)) != 0) {
                    flist_addf(failures, "expected error prefix %s, got %s", expected_err, err);
                }
                for (size_t k = 0; k < n; k++) event_free(&batch[k]);
            }
        } else if (!accepted) {
            flist_addf(failures, "event at index %zu was rejected unexpectedly: %s", i, err);
            for (size_t k = 0; k < n; k++) event_free(&batch[k]);
        }
        /* On (unexpected) acceptance with n>0, ownership already moved
         * into arb.history by arbiter_submit -- nothing to free here. */
    }

    State state = arbiter_state(&arb);
    const Json *expect = json_get(fixture, "expect");

    if (expect) {
        const Json *opt = json_get(expect, "option_space");
        if (opt) {
            size_t n = json_array_len(opt);
            ObjectId *want = malloc((n ? n : 1) * sizeof(ObjectId));
            for (size_t i = 0; i < n; i++) json_as_u64(json_array_get(opt, i), &want[i]);
            if (!objset_equals_array(&state.option_space, want, n)) {
                flist_addf(failures, "option_space: mismatch (expected %zu objects)", n);
            }
            free(want);
        }
        const Json *committed = json_get(expect, "committed");
        if (committed) {
            size_t n = json_array_len(committed);
            ObjectId *want = malloc((n ? n : 1) * sizeof(ObjectId));
            for (size_t i = 0; i < n; i++) json_as_u64(json_array_get(committed, i), &want[i]);
            if (!objset_equals_array(&state.committed, want, n)) {
                flist_addf(failures, "committed: mismatch (expected %zu objects)", n);
            }
            free(want);
        }
        const Json *refused_count = json_get(expect, "refused_count");
        if (refused_count) {
            uint64_t want;
            json_as_u64(refused_count, &want);
            if (state.refused.len != want) {
                flist_addf(failures, "refused_count: expected %llu, got %zu", (unsigned long long)want, state.refused.len);
            }
        }
        const Json *history_len = json_get(expect, "history_len");
        if (history_len) {
            uint64_t want;
            json_as_u64(history_len, &want);
            if (arbiter_len(&arb) != want) {
                flist_addf(failures, "history_len: expected %llu, got %zu", (unsigned long long)want, arbiter_len(&arb));
            }
        }
        const Json *observed_rules = json_get(expect, "observed_rules");
        if (observed_rules) {
            size_t n = json_array_len(observed_rules);
            if (n != state.observed.len) {
                flist_addf(failures, "observed_rules: length mismatch (expected %zu, got %zu)", n, state.observed.len);
            } else {
                for (size_t i = 0; i < n; i++) {
                    const char *want = json_as_str(json_array_get(observed_rules, i));
                    if (!want || strcmp(want, state.observed.items[i].rule) != 0) {
                        flist_addf(failures, "observed_rules[%zu]: expected %s, got %s", i, want ? want : "?", state.observed.items[i].rule);
                    }
                }
            }
        }
        const Json *bound = json_get(expect, "bound");
        if (bound) {
            size_t n = json_array_len(bound);
            for (size_t i = 0; i < n; i++) {
                const Json *triple = json_array_get(bound, i);
                uint64_t a, b;
                json_as_u64(json_array_get(triple, 0), &a);
                json_as_u64(json_array_get(triple, 1), &b);
                const char *tag = json_as_str(json_array_get(triple, 2));
                if (!boundlist_contains(&state.bound, a, b, tag)) {
                    flist_addf(failures, "bound: expected (%llu, %llu, %s) to be present",
                               (unsigned long long)a, (unsigned long long)b, tag ? tag : "");
                }
            }
        }
        const Json *qsc = json_get(expect, "quotient_same_class");
        if (qsc) {
            UnionFind uf = collapse_quotient(arb.history, arb.history_len);
            size_t n = json_array_len(qsc);
            for (size_t i = 0; i < n; i++) {
                const Json *triple = json_array_get(qsc, i);
                uint64_t a, b;
                json_as_u64(json_array_get(triple, 0), &a);
                json_as_u64(json_array_get(triple, 1), &b);
                bool want = json_as_bool(json_array_get(triple, 2), false);
                bool got = uf_same_class(&uf, a, b);
                if (got != want) {
                    flist_addf(failures, "quotient_same_class(%llu, %llu): expected %s, got %s",
                               (unsigned long long)a, (unsigned long long)b, want ? "true" : "false", got ? "true" : "false");
                }
            }
            uf_free(&uf);
        }
        const Json *qhr = json_get(expect, "quotient_honoring_refusals_same_class");
        if (qhr) {
            UnionFind uf = collapse_quotient_honoring_refusals(arb.history, arb.history_len);
            size_t n = json_array_len(qhr);
            for (size_t i = 0; i < n; i++) {
                const Json *triple = json_array_get(qhr, i);
                uint64_t a, b;
                json_as_u64(json_array_get(triple, 0), &a);
                json_as_u64(json_array_get(triple, 1), &b);
                bool want = json_as_bool(json_array_get(triple, 2), false);
                bool got = uf_same_class(&uf, a, b);
                if (got != want) {
                    flist_addf(failures, "quotient_honoring_refusals_same_class(%llu, %llu): expected %s, got %s",
                               (unsigned long long)a, (unsigned long long)b, want ? "true" : "false", got ? "true" : "false");
                }
            }
            uf_free(&uf);
        }
        const Json *meta_keys = json_get(expect, "meta_keys");
        if (meta_keys) {
            size_t n = json_array_len(meta_keys);
            for (size_t i = 0; i < n; i++) {
                uint64_t key;
                json_as_u64(json_array_get(meta_keys, i), &key);
                if (!collapse_meta_has_key(arb.history, arb.history_len, key)) {
                    flist_addf(failures, "meta_keys: expected object %llu to have metadata", (unsigned long long)key);
                }
            }
        }
        if (json_get_bool(expect, "deterministic_replay", false)) {
            State s1 = arbiter_state(&arb);
            State s2 = arbiter_state(&arb);
            if (!state_equals(&s1, &s2)) {
                flist_addf(failures, "deterministic_replay: two replays of the same history disagreed");
            }
            state_free(&s1);
            state_free(&s2);
        }
    }

    state_free(&state);
    arbiter_free(&arb);

    return failures->len > 0 ? OUTCOME_FAIL : OUTCOME_PASS;
}

static Outcome run_fixture(const char *path, FailureList *failures) {
    FILE *f = fopen(path, "rb");
    if (!f) { flist_addf(failures, "cannot open file"); return OUTCOME_FAIL; }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(sz + 1);
    size_t nread = fread(buf, 1, sz, f);
    buf[nread] = '\0';
    fclose(f);

    char *err = NULL;
    Json *fixture = json_parse(buf, &err);
    free(buf);
    if (!fixture) {
        flist_addf(failures, "json parse error: %s", err);
        json_free_error(err);
        return OUTCOME_FAIL;
    }

    Outcome outcome;
    if (json_get(fixture, "history_a")) {
        outcome = run_meld_fixture(fixture, failures);
    } else if (json_get_bool(fixture, "manual", false)) {
        outcome = OUTCOME_MANUAL_SKIP;
        const char *required[] = {"invariant", "explanation"};
        for (size_t i = 0; i < 2; i++) {
            if (!json_get(fixture, required[i])) {
                flist_addf(failures, "manual fixture missing '%s'", required[i]);
                outcome = OUTCOME_FAIL;
            }
        }
    } else {
        outcome = run_executable_fixture(fixture, failures);
    }

    json_free(fixture);
    return outcome;
}

/* ---------------- directory listing ---------------- */

static int cmp_str(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

static size_t list_json_files(const char *dir, char ***out_paths) {
    DIR *d = opendir(dir);
    if (!d) {
        fprintf(stderr, "cannot read fixtures dir %s\n", dir);
        exit(1);
    }
    size_t cap = 16, n = 0;
    char **paths = malloc(cap * sizeof(char *));
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        const char *name = ent->d_name;
        size_t len = strlen(name);
        if (len < 6 || strcmp(name + len - 5, ".json") != 0) continue;
        char *full = malloc(strlen(dir) + 1 + len + 1);
        sprintf(full, "%s/%s", dir, name);
        if (n >= cap) { cap *= 2; paths = realloc(paths, cap * sizeof(char *)); }
        paths[n++] = full;
    }
    closedir(d);
    qsort(paths, n, sizeof(char *), cmp_str);
    *out_paths = paths;
    return n;
}

static const char *base_stem(const char *path) {
    const char *slash = strrchr(path, '/');
    return slash ? slash + 1 : path;
}

int main(int argc, char **argv) {
    const char *dir = argc > 1 ? argv[1] : FIXTURES_DEFAULT_DIR;

    char **paths;
    size_t n = list_json_files(dir, &paths);

    size_t pass = 0, fail = 0, manual = 0;
    for (size_t i = 0; i < n; i++) {
        FailureList failures;
        flist_init(&failures);
        Outcome outcome = run_fixture(paths[i], &failures);

        const char *stem = base_stem(paths[i]);
        char name[256];
        snprintf(name, sizeof(name), "%s", stem);
        char *dot = strstr(name, ".json");
        if (dot) *dot = '\0';

        switch (outcome) {
            case OUTCOME_PASS:
                printf("PASS  %s\n", name);
                pass++;
                break;
            case OUTCOME_MANUAL_SKIP:
                printf("SKIP  %s (manual/structural only)\n", name);
                manual++;
                break;
            case OUTCOME_FAIL:
                printf("FAIL  %s\n", name);
                for (size_t k = 0; k < failures.len; k++) printf("      - %s\n", failures.items[k]);
                fail++;
                break;
        }
        flist_free(&failures);
        free(paths[i]);
    }
    free(paths);

    printf("\n%zu passed, %zu failed, %zu manual, %zu total\n", pass, fail, manual, pass + fail + manual);
    return fail > 0 ? 1 : 0;
}
