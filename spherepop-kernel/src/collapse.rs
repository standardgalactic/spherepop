//! Collapse rules: `c : History -> O_c`.
//!
//! Collapse rules are pure functions of `History`, external to `apply`.
//! `collapse_quotient` realizes Merge-sugar: `Merge_c(a,b) :=
//! Collapse_c(Bind(a,b))` is exactly "observe H under a rule that
//! identifies Bind-connected elements."

use crate::event::{EventKind, ObjectId};
use crate::history::History;
use std::collections::HashMap;

/// A minimal union-find over `ObjectId`, used as the observational space
/// `O_c` for the quotienting collapse rule.
#[derive(Default, Debug, Clone)]
pub struct UnionFind {
    parent: HashMap<ObjectId, ObjectId>,
}

impl UnionFind {
    pub fn new() -> Self {
        UnionFind::default()
    }

    fn root(&mut self, x: ObjectId) -> ObjectId {
        let p = *self.parent.entry(x).or_insert(x);
        if p == x {
            x
        } else {
            let r = self.root(p);
            self.parent.insert(x, r);
            r
        }
    }

    pub fn union(&mut self, a: ObjectId, b: ObjectId) {
        let ra = self.root(a);
        let rb = self.root(b);
        if ra != rb {
            self.parent.insert(ra, rb);
        }
    }

    /// Whether `a` and `b` are in the same equivalence class — i.e.
    /// whether the Merge-sugar quotient identifies them.
    pub fn same_class(&mut self, a: ObjectId, b: ObjectId) -> bool {
        self.root(a) == self.root(b)
    }
}

/// c_~ : identifies objects connected by a Bind. This IS Merge, per the
/// specification's Def. of Merge: `Merge_c(a,b) := Collapse_c(Bind(a,b))`.
pub fn collapse_quotient(h: &History) -> UnionFind {
    let mut uf = UnionFind::new();
    for e in h.events_of_kind(EventKind::Bind) {
        if e.tag.as_deref() == Some("__meta__") {
            continue; // ordinary rules are blind to metadata bindings by convention
        }
        uf.union(e.a.unwrap(), e.b.unwrap());
    }
    uf
}

/// c_~ honoring withdrawn bindings: a quotient rule that excludes any
/// Bind pair later Refuse'd (Unlink-sugar) from the identification.
/// Demonstrates that a collapse rule MAY choose to respect refusals —
/// this is a policy decision for the rule, not a structural deletion.
pub fn collapse_quotient_honoring_refusals(h: &History) -> UnionFind {
    let mut uf = UnionFind::new();
    let withdrawn: std::collections::HashSet<(ObjectId, ObjectId)> = h
        .events_of_kind(EventKind::Refuse)
        .filter(|e| e.reason.as_deref() == Some("relation withdrawn"))
        .filter_map(|e| e.a.zip(e.b))
        .collect();
    for e in h.events_of_kind(EventKind::Bind) {
        if e.tag.as_deref() == Some("__meta__") {
            continue;
        }
        let a = e.a.unwrap();
        let b = e.b.unwrap();
        if !withdrawn.contains(&(a, b)) && !withdrawn.contains(&(b, a)) {
            uf.union(a, b);
        }
    }
    uf
}

/// c_meta : the distinguished rule other rules are defined to ignore
/// (SetMeta-sugar). Reads Bind events tagged "__meta__" as key/value
/// annotations rather than ordinary relations.
pub fn collapse_meta(h: &History) -> HashMap<ObjectId, Vec<String>> {
    let mut out: HashMap<ObjectId, Vec<String>> = HashMap::new();
    for e in h.events_of_kind(EventKind::Bind) {
        if e.tag.as_deref() == Some("__meta__") {
            out.entry(e.a.unwrap()).or_default().push(format!("{:?}", e.b));
        }
    }
    out
}

/// c_I : the identity/finest rule — every event visible, nothing quotiented.
pub fn collapse_identity(h: &History) -> &[crate::event::Event] {
    h.as_slice()
}
