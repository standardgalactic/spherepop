//! Desugaring into the four primitives (specification, "The Derived
//! Surface Calculus"). None of these are events in their own right —
//! each returns a `Vec<Event>` built entirely from `Pop`/`Refuse`/`Bind`/
//! `Collapse`.

use crate::event::{Event, ObjectId, RuleId};

/// Link(a,b) := Bind(a,b), exactly.
pub fn link(a: ObjectId, b: ObjectId, relation: impl Into<String>) -> Event {
    Event::bind(a, b, relation)
}

/// Unlink(a,b) := Refuse(Bind(a,b)). Does not remove the original Bind
/// event — only documents that reliance on it is withdrawn.
pub fn unlink(a: ObjectId, b: ObjectId) -> Event {
    Event::refuse_bind(a, b, "relation withdrawn")
}

/// Choice(A,B) := Pop(A) || Refuse(B): committing to A while explicitly,
/// auditably refusing B, rather than letting the untaken branch vanish.
pub fn choice(taken: ObjectId, rejected: ObjectId) -> Vec<Event> {
    vec![Event::pop(taken), Event::refuse(rejected, "not selected by Choice")]
}

/// Merge_c(a,b) := Collapse_c(Bind(a,b)): a Bind followed by an
/// observation under an identification-quotient rule `c`. Returns the two
/// events to submit; the caller invokes the matching rule (e.g.
/// `collapse::collapse_quotient`) to read back the resulting classes.
pub fn merge(a: ObjectId, b: ObjectId, rule: RuleId) -> Vec<Event> {
    vec![Event::bind(a, b, "merge"), Event::collapse(rule)]
}

/// SetMeta(o,k,v) := Bind(o,(k,v)) under a distinguished metadata rule
/// every other rule is defined to ignore. `k` is folded into the Bind tag
/// as a `"__meta__"` marker so `collapse::collapse_meta` can find it and
/// ordinary collapse rules (e.g. `collapse_quotient`) skip it.
pub fn set_meta(object: ObjectId, key_as_object_id: ObjectId) -> Event {
    Event::bind(object, key_as_object_id, "__meta__")
}
