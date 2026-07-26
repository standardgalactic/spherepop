//! The event alphabet E = { Pop, Refuse, Bind, Collapse }.
//!
//! Per the specification: these are the ONLY primitive events. Sphere,
//! Merge, Choice, Link, Unlink, Nest, and SetMeta are surface-calculus
//! sugar built from these four (see `sugar.rs`) and never appear as
//! their own `EventKind` variant.

pub type ObjectId = u64;
pub type LogPos = u64;
pub type RuleId = &'static str;

#[repr(u8)]
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub enum EventKind {
    /// Commitment: remove `a` from the option space, foreclosing alternatives.
    Pop = 0,
    /// Documented inadmissibility of `a`, with `reason`. Never touches Omega.
    Refuse = 1,
    /// Coupling of `(a, b)`, optionally tagged with a relation label.
    /// Never identifies `a` and `b` — relatedness is not identity.
    Bind = 2,
    /// Observation of H under a named, admissibility-certified collapse rule.
    Collapse = 3,
}

/// A single primitive event. Every field beyond `kind` and `pos` is
/// `Option`-typed because different kinds populate different subsets —
/// this mirrors the ABI note in the specification: the layout is additive
/// and a decoder should treat absent fields as "not applicable to this kind"
/// rather than as malformed data.
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Event {
    pub kind: EventKind,
    pub pos: LogPos,
    /// Pop's `x`; Bind's first element; Refuse's target object (if any).
    pub a: Option<ObjectId>,
    /// Bind's second element.
    pub b: Option<ObjectId>,
    /// Bind's relation label (Link-sugar and SetMeta-sugar both use this).
    pub tag: Option<String>,
    /// Refuse's rationale. A refusal without a reason documents nothing,
    /// per the specification — constructors below enforce this.
    pub reason: Option<String>,
    /// Collapse's rule identifier `c`.
    pub rule: Option<RuleId>,
}

impl Event {
    pub fn pop(x: ObjectId) -> Self {
        Event { kind: EventKind::Pop, pos: 0, a: Some(x), b: None, tag: None, reason: None, rule: None }
    }

    pub fn refuse(x: ObjectId, reason: impl Into<String>) -> Self {
        Event { kind: EventKind::Refuse, pos: 0, a: Some(x), b: None, tag: None, reason: Some(reason.into()), rule: None }
    }

    /// Refuse targeting an existing Bind pair (Unlink-sugar):
    /// Unlink(a,b) := Refuse(Bind(a,b)).
    pub fn refuse_bind(a: ObjectId, b: ObjectId, reason: impl Into<String>) -> Self {
        Event { kind: EventKind::Refuse, pos: 0, a: Some(a), b: Some(b), tag: None, reason: Some(reason.into()), rule: None }
    }

    pub fn bind(a: ObjectId, b: ObjectId, tag: impl Into<String>) -> Self {
        Event { kind: EventKind::Bind, pos: 0, a: Some(a), b: Some(b), tag: Some(tag.into()), reason: None, rule: None }
    }

    pub fn collapse(rule: RuleId) -> Self {
        Event { kind: EventKind::Collapse, pos: 0, a: None, b: None, tag: None, reason: None, rule: Some(rule) }
    }
}
