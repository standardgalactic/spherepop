//! The Arbiter: the only path by which a `History` is ever extended.
//!
//! `validate` reads `State` — which carries only `(H, Omega)`-level facts —
//! and never the value any collapse rule would compute. There is no
//! parameter through which `c(H)` could be passed in, for any `c`: this is
//! Requirement (Observation Non-Interference) enforced by the type
//! signature, not by convention.

use crate::event::{Event, EventKind, LogPos, ObjectId, RuleId};
use crate::history::{History, State};
use std::collections::HashSet;

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum ArbiterError {
    Malformed(String),
    PopOutsideOptionSpace(ObjectId),
    UncertifiedCollapseRule(RuleId),
    RefuseWithoutReason,
    StaleOverlay,
}

impl std::fmt::Display for ArbiterError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{:?}", self)
    }
}
impl std::error::Error for ArbiterError {}

pub struct Proposal {
    pub events: Vec<Event>,
}

impl Proposal {
    pub fn new(events: Vec<Event>) -> Self {
        Proposal { events }
    }
}

pub struct Arbiter {
    history: History,
    /// The admissibility-certified rule registry: an uncertified rule
    /// name is a type error at proposal time, matching the primitive
    /// Collapse definition's precondition Gamma |- t : Adm(T).
    rules: HashSet<RuleId>,
    omega_0: HashSet<ObjectId>,
}

impl Arbiter {
    pub fn new(omega_0: impl IntoIterator<Item = ObjectId>, rules: impl IntoIterator<Item = RuleId>) -> Self {
        Arbiter {
            history: History::new(),
            rules: rules.into_iter().collect(),
            omega_0: omega_0.into_iter().collect(),
        }
    }

    pub fn len(&self) -> usize {
        self.history.len()
    }

    pub fn history_ref(&self) -> &History {
        &self.history
    }

    pub fn history_clone(&self) -> History {
        self.history.clone()
    }

    pub fn state(&self) -> State {
        self.history.replay(&self.omega_0)
    }

    pub fn omega_0_ref(&self) -> &HashSet<ObjectId> {
        &self.omega_0
    }

    /// Validates and, if accepted, appends a proposal to H. The ONLY path
    /// by which H is ever extended.
    pub fn submit(&mut self, p: Proposal) -> Result<Vec<LogPos>, ArbiterError> {
        self.validate(&p.events)?;
        let mut positions = Vec::new();
        for mut e in p.events {
            let pos = self.history.len() as LogPos;
            e.pos = pos;
            self.history.push(e);
            positions.push(pos);
        }
        Ok(positions)
    }

    fn validate(&self, events: &[Event]) -> Result<(), ArbiterError> {
        // Structural state only: (H, Omega)-level facts, never c(H) for any c.
        let s = self.history.replay(&self.omega_0);
        let mut hypothetical_committed: HashSet<ObjectId> = HashSet::new();

        for e in events {
            match e.kind {
                EventKind::Pop => {
                    let x = e.a.ok_or_else(|| ArbiterError::Malformed("Pop missing a".into()))?;
                    let still_available = s.option_space.contains(&x) && !hypothetical_committed.contains(&x);
                    if !still_available {
                        return Err(ArbiterError::PopOutsideOptionSpace(x));
                    }
                    hypothetical_committed.insert(x);
                }
                EventKind::Refuse => {
                    if e.reason.as_deref().unwrap_or("").is_empty() {
                        return Err(ArbiterError::RefuseWithoutReason);
                    }
                }
                EventKind::Bind => {
                    if e.a.is_none() || e.b.is_none() {
                        return Err(ArbiterError::Malformed("Bind missing a/b".into()));
                    }
                }
                EventKind::Collapse => {
                    let rule = e.rule.ok_or_else(|| ArbiterError::Malformed("Collapse missing rule".into()))?;
                    if !self.rules.contains(rule) {
                        return Err(ArbiterError::UncertifiedCollapseRule(rule));
                    }
                }
            }
        }
        Ok(())
    }
}
