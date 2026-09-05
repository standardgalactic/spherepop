//! Flat fixture conformance runner.
//!
//! Loads every `*.json` fixture from `experiments/flat/fixtures/` (path
//! overridable via the first CLI argument), replays its event sequence
//! through the reference `Arbiter`, and checks the resulting state
//! against the fixture's `expect` block. This is the Rust side of the
//! two-implementation conformance check described in the tracking issue;
//! `experiments/flat/run_python.py` is an independent oracle that reads
//! the exact same fixture files.
//!
//! Usage:
//!   cargo run --bin fixtures [path/to/fixtures/dir]

use spherepop_kernel::json::Json;
use spherepop_kernel::{collapse, sugar, Arbiter, ArbiterError, Event, Proposal};
use std::collections::{BTreeSet, HashSet};
use std::path::{Path, PathBuf};

struct Failure(String);

fn expect_events_from_op(op: &str, e: &Json) -> Vec<Event> {
    match op {
        "pop" => vec![Event::pop(e.field("a").as_u64().unwrap())],
        "refuse" => vec![Event::refuse(
            e.field("a").as_u64().unwrap(),
            e.get("reason").and_then(Json::as_str).unwrap_or("").to_string(),
        )],
        "refuse_bind" => vec![Event::refuse_bind(
            e.field("a").as_u64().unwrap(),
            e.field("b").as_u64().unwrap(),
            e.get("reason").and_then(Json::as_str).unwrap_or("").to_string(),
        )],
        "bind" => vec![Event::bind(
            e.field("a").as_u64().unwrap(),
            e.field("b").as_u64().unwrap(),
            e.get("tag").and_then(Json::as_str).unwrap_or("").to_string(),
        )],
        "collapse" => vec![Event::collapse(Box::leak(
            e.field("rule").as_str().unwrap().to_string().into_boxed_str(),
        ))],
        "link" => vec![sugar::link(
            e.field("a").as_u64().unwrap(),
            e.field("b").as_u64().unwrap(),
            e.get("tag").and_then(Json::as_str).unwrap_or("").to_string(),
        )],
        "unlink" => vec![sugar::unlink(e.field("a").as_u64().unwrap(), e.field("b").as_u64().unwrap())],
        "choice" => sugar::choice(e.field("taken").as_u64().unwrap(), e.field("rejected").as_u64().unwrap()),
        "merge" => sugar::merge(
            e.field("a").as_u64().unwrap(),
            e.field("b").as_u64().unwrap(),
            Box::leak(e.field("rule").as_str().unwrap().to_string().into_boxed_str()),
        ),
        "set_meta" => vec![sugar::set_meta(e.field("object").as_u64().unwrap(), e.field("key").as_u64().unwrap())],
        other => panic!("unknown fixture op {:?}", other),
    }
}

fn error_matches(err: &ArbiterError, expected: &str) -> bool {
    format!("{:?}", err).starts_with(expected)
}

enum RunOutcome {
    Pass,
    ManualSkip,
    Fail(Vec<Failure>),
}

fn run_fixture(path: &Path) -> RunOutcome {
    let text = match std::fs::read_to_string(path) {
        Ok(t) => t,
        Err(e) => return RunOutcome::Fail(vec![Failure(format!("read error: {e}"))]),
    };
    let fixture = match spherepop_kernel::json::parse(&text) {
        Ok(v) => v,
        Err(e) => return RunOutcome::Fail(vec![Failure(format!("json parse error: {e}"))]),
    };

    if fixture.get("manual").and_then(Json::as_bool).unwrap_or(false) {
        // Structural-only fixture (e.g. Meld): confirm required narrative
        // fields are present, but do not execute it through the Arbiter.
        for required in ["invariant", "explanation"] {
            if fixture.get(required).is_none() {
                return RunOutcome::Fail(vec![Failure(format!("manual fixture missing {:?}", required))]);
            }
        }
        return RunOutcome::ManualSkip;
    }

    match run_executable_fixture(&fixture) {
        Ok(()) => RunOutcome::Pass,
        Err(f) => RunOutcome::Fail(f),
    }
}

fn run_executable_fixture(fixture: &Json) -> Result<(), Vec<Failure>> {
    let omega0: BTreeSet<u64> = fixture
        .field("initial_option_space")
        .as_array()
        .unwrap()
        .iter()
        .map(|v| v.as_u64().unwrap())
        .collect();
    let rules: Vec<&'static str> = fixture
        .get("certified_rules")
        .and_then(Json::as_array)
        .unwrap_or(&[])
        .iter()
        .map(|v| Box::leak(v.as_str().unwrap().to_string().into_boxed_str()) as &'static str)
        .collect();

    let omega0_hashset: HashSet<u64> = omega0.iter().copied().collect();
    let mut arb = Arbiter::new(omega0.iter().copied(), rules);
    let mut failures = Vec::new();

    for ev in fixture.get("events").and_then(Json::as_array).unwrap_or(&[]) {
        let op = ev.field("op").as_str().unwrap();
        let events = expect_events_from_op(op, ev);
        let expect_reject = ev.get("expect_reject").and_then(Json::as_bool).unwrap_or(false);
        let len_before = arb.len();
        let result = arb.submit(Proposal::new(events));
        if expect_reject {
            match result {
                Ok(_) => failures.push(Failure(format!("event {:?} was accepted but fixture expected rejection", ev))),
                Err(e) => {
                    if arb.len() != len_before {
                        failures.push(Failure("rejected event mutated history length".into()));
                    }
                    if let Some(expected_err) = ev.get("expect_error").and_then(Json::as_str) {
                        if !error_matches(&e, expected_err) {
                            failures.push(Failure(format!("expected error prefix {:?}, got {:?}", expected_err, e)));
                        }
                    }
                }
            }
        } else if let Err(e) = result {
            failures.push(Failure(format!("event {:?} was rejected unexpectedly: {:?}", ev, e)));
        }
    }

    let state = arb.state();
    if let Some(expect) = fixture.get("expect") {
        if let Some(arr) = expect.get("option_space").and_then(Json::as_array) {
            let expected: BTreeSet<u64> = arr.iter().map(|v| v.as_u64().unwrap()).collect();
            let actual: BTreeSet<u64> = state.option_space.iter().copied().collect();
            if actual != expected {
                failures.push(Failure(format!("option_space: expected {:?}, got {:?}", expected, actual)));
            }
        }
        if let Some(arr) = expect.get("committed").and_then(Json::as_array) {
            let expected: BTreeSet<u64> = arr.iter().map(|v| v.as_u64().unwrap()).collect();
            let actual: BTreeSet<u64> = state.committed.iter().copied().collect();
            if actual != expected {
                failures.push(Failure(format!("committed: expected {:?}, got {:?}", expected, actual)));
            }
        }
        if let Some(n) = expect.get("refused_count").and_then(Json::as_u64) {
            if state.refused.len() as u64 != n {
                failures.push(Failure(format!("refused_count: expected {}, got {}", n, state.refused.len())));
            }
        }
        if let Some(n) = expect.get("history_len").and_then(Json::as_u64) {
            if arb.len() as u64 != n {
                failures.push(Failure(format!("history_len: expected {}, got {}", n, arb.len())));
            }
        }
        if let Some(arr) = expect.get("observed_rules").and_then(Json::as_array) {
            let expected: Vec<String> = arr.iter().map(|v| v.as_str().unwrap().to_string()).collect();
            let actual: Vec<String> = state.observed.iter().map(|(_, r)| r.to_string()).collect();
            if actual != expected {
                failures.push(Failure(format!("observed_rules: expected {:?}, got {:?}", expected, actual)));
            }
        }
        if let Some(arr) = expect.get("bound").and_then(Json::as_array) {
            for triple in arr {
                let t = triple.as_array().unwrap();
                let a = t[0].as_u64().unwrap();
                let b = t[1].as_u64().unwrap();
                let tag = t[2].as_str().unwrap();
                if !state.bound.contains(&(a, b, tag.to_string())) {
                    failures.push(Failure(format!("bound: expected ({}, {}, {:?}) to be present", a, b, tag)));
                }
            }
        }
        if let Some(arr) = expect.get("quotient_same_class").and_then(Json::as_array) {
            let mut classes = collapse::collapse_quotient(arb.history_ref());
            for triple in arr {
                let t = triple.as_array().unwrap();
                let a = t[0].as_u64().unwrap();
                let b = t[1].as_u64().unwrap();
                let expected = t[2].as_bool().unwrap();
                let actual = classes.same_class(a, b);
                if actual != expected {
                    failures.push(Failure(format!("quotient_same_class({}, {}): expected {}, got {}", a, b, expected, actual)));
                }
            }
        }
        if let Some(arr) = expect.get("quotient_honoring_refusals_same_class").and_then(Json::as_array) {
            let mut classes = collapse::collapse_quotient_honoring_refusals(arb.history_ref());
            for triple in arr {
                let t = triple.as_array().unwrap();
                let a = t[0].as_u64().unwrap();
                let b = t[1].as_u64().unwrap();
                let expected = t[2].as_bool().unwrap();
                let actual = classes.same_class(a, b);
                if actual != expected {
                    failures.push(Failure(format!(
                        "quotient_honoring_refusals_same_class({}, {}): expected {}, got {}",
                        a, b, expected, actual
                    )));
                }
            }
        }
        if let Some(arr) = expect.get("meta_keys").and_then(Json::as_array) {
            let meta = collapse::collapse_meta(arb.history_ref());
            for k in arr {
                let key = k.as_u64().unwrap();
                if !meta.contains_key(&key) {
                    failures.push(Failure(format!("meta_keys: expected object {} to have metadata", key)));
                }
            }
        }
        if expect.get("deterministic_replay").and_then(Json::as_bool).unwrap_or(false) {
            let s1 = arb.history_ref().replay(&omega0_hashset);
            let s2 = arb.history_ref().replay(&omega0_hashset);
            if s1 != s2 {
                failures.push(Failure("deterministic_replay: two replays of the same history disagreed".into()));
            }
        }
    }

    if failures.is_empty() {
        Ok(())
    } else {
        Err(failures)
    }
}

fn default_fixtures_dir() -> PathBuf {
    // spherepop-kernel/ is a sibling of experiments/ at the repo root.
    Path::new(env!("CARGO_MANIFEST_DIR")).join("../experiments/flat/fixtures")
}

fn main() {
    let dir = std::env::args().nth(1).map(PathBuf::from).unwrap_or_else(default_fixtures_dir);

    let mut entries: Vec<PathBuf> = std::fs::read_dir(&dir)
        .unwrap_or_else(|e| panic!("cannot read fixtures dir {:?}: {}", dir, e))
        .filter_map(|e| e.ok())
        .map(|e| e.path())
        .filter(|p| p.extension().map(|ext| ext == "json").unwrap_or(false))
        .collect();
    entries.sort();

    let mut pass = 0usize;
    let mut fail = 0usize;
    let mut manual = 0usize;
    for path in &entries {
        let name = path.file_stem().unwrap().to_string_lossy().to_string();
        match run_fixture(path) {
            RunOutcome::Pass => {
                println!("PASS  {}", name);
                pass += 1;
            }
            RunOutcome::ManualSkip => {
                println!("SKIP  {} (manual/structural only)", name);
                manual += 1;
            }
            RunOutcome::Fail(failures) => {
                println!("FAIL  {}", name);
                for f in failures {
                    println!("      - {}", f.0);
                }
                fail += 1;
            }
        }
    }

    println!("\n{} passed, {} failed, {} manual, {} total", pass, fail, manual, pass + fail + manual);
    if fail > 0 {
        std::process::exit(1);
    }
}
