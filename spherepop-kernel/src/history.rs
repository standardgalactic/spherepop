//! Histories, the option space, and deterministic replay.
//!
//! `History` is the free monoid over the event alphabet (append, `++`,
//! empty). `apply` is the per-event transition function whose fold over
//! a history is `Replay`. `apply` is deliberately a pure function of
//! `(State, Event)` alone — no wall-clock reads, no ambient globals — so
//! that Invariant (Deterministic Replay) is checkable rather than merely
//! asserted: replaying the same `History` twice is guaranteed by
//! construction to produce equal `State` values.

use crate::event::{Event, EventKind, LogPos, ObjectId, RuleId};
use std::collections::HashSet;

#[derive(Default, Clone, Debug, PartialEq, Eq)]
pub struct State {
    /// Omega: the current option space.
    pub option_space: HashSet<ObjectId>,
    /// Symbols already committed via Pop.
    pub committed: HashSet<ObjectId>,
    /// Bind facts: (a, b, tag).
    pub bound: HashSet<(ObjectId, ObjectId, String)>,
    /// (position, target, reason) for every Refuse event, target=0 for
    /// refusals that target a Bind pair rather than a bare object (see
    /// Unlink-sugar, which is Refuse(Bind(a,b))).
    pub refused: Vec<(LogPos, Option<ObjectId>, String)>,
    /// Audit trail of Collapse invocations: (position, rule). Deliberately
    /// does NOT store the observed value c(H) itself — see Requirement
    /// (Observation Non-Interference) in the specification.
    pub observed: Vec<(LogPos, RuleId)>,
}

#[derive(Default, Clone, Debug, PartialEq, Eq)]
pub struct History {
    events: Vec<Event>,
}

impl History {
    pub fn new() -> Self {
        History::default()
    }

    pub fn len(&self) -> usize {
        self.events.len()
    }

    pub fn is_empty(&self) -> bool {
        self.events.is_empty()
    }

    pub fn as_slice(&self) -> &[Event] {
        &self.events
    }

    /// The only mutator: append. There is no remove/undo, matching the
    /// free-monoid structure and the Irreversibility corollary.
    pub(crate) fn push(&mut self, event: Event) {
        self.events.push(event);
    }

    pub fn events_of_kind(&self, kind: EventKind) -> impl Iterator<Item = &Event> {
        self.events.iter().filter(move |e| e.kind == kind)
    }

    /// Meld: parallel composition of two independently-generated histories
    /// (the free monoidal tensor). Structural fact about the history
    /// monoid; NOT required by the concurrency clause of the Completeness
    /// Theorem, which goes through Bind alone. Provided here because it is
    /// still a true, useful operation for combining two logs.
    pub fn meld(&mut self, other: &History) {
        self.events.extend(other.events.iter().cloned());
    }

    /// Replay: fold `apply` over the history from a given initial Omega.
    pub fn replay(&self, omega_0: &HashSet<ObjectId>) -> State {
        let mut s = State {
            option_space: omega_0.clone(),
            ..State::default()
        };
        for e in &self.events {
            apply(&mut s, e);
        }
        s
    }
}

pub fn apply(s: &mut State, e: &Event) {
    match e.kind {
        EventKind::Pop => {
            let x = e.a.expect("Pop event missing target `a`");
            s.option_space.remove(&x); // |Omega| decreases by exactly one
            s.committed.insert(x);
        }
        EventKind::Refuse => {
            s.refused.push((e.pos, e.a, e.reason.clone().unwrap_or_default()));
            // Omega is untouched: refusal documents, it does not foreclose.
        }
        EventKind::Bind => {
            let a = e.a.expect("Bind event missing `a`");
            let b = e.b.expect("Bind event missing `b`");
            s.bound.insert((a, b, e.tag.clone().unwrap_or_default()));
        }
        EventKind::Collapse => {
            // Collapse never mutates committed/bound/refused, and its
            // observable value c(H) is computed separately (see
            // `collapse.rs`), never stored in State itself.
            let rule = e.rule.expect("Collapse event missing `rule`");
            s.observed.push((e.pos, rule));
        }
    }
}

/// The event-weight function w and the generalised possibility functional
/// Pi(H, Omega) = |Omega| + sum_e w(e), used by the Conservation Law test.
pub fn possibility_functional(state: &State) -> usize {
    let popped = state.committed.len(); // w(Pop) = 1, all other kinds w = 0
    state.option_space.len() + popped
}
