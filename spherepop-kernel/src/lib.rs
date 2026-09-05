//! # spherepop-kernel
//!
//! Reference kernel for the Spherepop four-primitive algebra: `Pop`,
//! `Refuse`, `Bind`, `Collapse`. Implements the World model `(H, Omega)`,
//! the Arbiter (the sole path by which `H` is extended), collapse rules
//! (pure observation functions, one of which realizes Merge-sugar), and
//! an overlay manager with a preview-then-commit workflow that has no
//! auto-commit code path.
//!
//! Every other operator in the specification — Sphere, Merge, Choice,
//! Link, Unlink, Nest, SetMeta — is surface-calculus sugar, provided in
//! `sugar` as functions returning `Vec<Event>` built entirely from the
//! four primitives; none of them is its own `EventKind` variant.

pub mod arbiter;
pub mod collapse;
pub mod event;
pub mod history;
pub mod json;
pub mod overlay;
pub mod sugar;
pub mod wire;

pub use arbiter::{Arbiter, ArbiterError, Proposal};
pub use event::{Event, EventKind, LogPos, ObjectId, RuleId};
pub use history::{apply, possibility_functional, History, State};
pub use overlay::{Overlay, OverlayManager};

#[cfg(test)]
mod tests {
    use super::*;
    use std::collections::HashSet;

    fn omega(items: &[ObjectId]) -> HashSet<ObjectId> {
        items.iter().copied().collect()
    }

    // ---- Primitive-level behavior -----------------------------------

    #[test]
    fn pop_commits_and_shrinks_option_space() {
        let mut arb = Arbiter::new(omega(&[1, 2, 3]), []);
        arb.submit(Proposal::new(vec![Event::pop(1)])).unwrap();
        let s = arb.state();
        assert!(!s.option_space.contains(&1));
        assert!(s.committed.contains(&1));
        assert!(s.option_space.contains(&2) && s.option_space.contains(&3));
    }

    #[test]
    fn pop_outside_option_space_is_rejected() {
        let mut arb = Arbiter::new(omega(&[1, 2]), []);
        let err = arb.submit(Proposal::new(vec![Event::pop(99)])).unwrap_err();
        assert_eq!(err, ArbiterError::PopOutsideOptionSpace(99));
    }

    #[test]
    fn popping_the_same_symbol_twice_is_rejected() {
        let mut arb = Arbiter::new(omega(&[1]), []);
        arb.submit(Proposal::new(vec![Event::pop(1)])).unwrap();
        let err = arb.submit(Proposal::new(vec![Event::pop(1)])).unwrap_err();
        assert_eq!(err, ArbiterError::PopOutsideOptionSpace(1));
    }

    #[test]
    fn refuse_does_not_touch_option_space() {
        let mut arb = Arbiter::new(omega(&[1, 2]), []);
        arb.submit(Proposal::new(vec![Event::refuse(1, "inadmissible for now")]))
            .unwrap();
        let s = arb.state();
        // Refusal is documented...
        assert_eq!(s.refused.len(), 1);
        // ...but Omega is untouched (Refuse contracts admissibility, not Omega).
        assert!(s.option_space.contains(&1));
    }

    #[test]
    fn refuse_without_reason_is_rejected() {
        let mut arb = Arbiter::new(omega(&[1]), []);
        let err = arb.submit(Proposal::new(vec![Event::refuse(1, "")])).unwrap_err();
        assert_eq!(err, ArbiterError::RefuseWithoutReason);
    }

    #[test]
    fn bind_couples_without_identifying() {
        let mut arb = Arbiter::new(omega(&[1, 2]), []);
        arb.submit(Proposal::new(vec![Event::bind(1, 2, "adjacent")]))
            .unwrap();
        let s = arb.state();
        assert!(s.bound.contains(&(1, 2, "adjacent".to_string())));
        // Binding does not commit either element.
        assert!(s.option_space.contains(&1) && s.option_space.contains(&2));
    }

    #[test]
    fn collapse_with_uncertified_rule_is_rejected() {
        let mut arb = Arbiter::new(omega(&[1, 2]), ["identity"]); // "merge" not certified
        let err = arb
            .submit(Proposal::new(vec![Event::collapse("merge")]))
            .unwrap_err();
        assert_eq!(err, ArbiterError::UncertifiedCollapseRule("merge"));
    }

    #[test]
    fn collapse_with_certified_rule_is_recorded_but_state_stores_no_observed_value() {
        let mut arb = Arbiter::new(omega(&[1, 2]), ["identity"]);
        arb.submit(Proposal::new(vec![Event::collapse("identity")]))
            .unwrap();
        let s = arb.state();
        assert_eq!(s.observed, vec![(0, "identity")]);
        // No field of State carries a computed observation value.
    }

    // ---- Sugar / desugaring ------------------------------------------

    #[test]
    fn link_is_literally_bind() {
        assert_eq!(sugar::link(1, 2, "rel"), Event::bind(1, 2, "rel"));
    }

    #[test]
    fn unlink_leaves_the_original_bind_intact() {
        let mut arb = Arbiter::new(omega(&[1, 2]), []);
        arb.submit(Proposal::new(vec![sugar::link(1, 2, "adjacent")]))
            .unwrap();
        arb.submit(Proposal::new(vec![sugar::unlink(1, 2)])).unwrap();

        let s = arb.state();
        // Irreversibility: the bind is never deleted...
        assert!(s.bound.contains(&(1, 2, "adjacent".to_string())));
        // ...only a refusal documenting withdrawal is added.
        assert!(s
            .refused
            .iter()
            .any(|(_, _, r)| r == "relation withdrawn"));
    }

    #[test]
    fn choice_commits_taken_and_refuses_rejected() {
        let mut arb = Arbiter::new(omega(&[10, 20]), []);
        arb.submit(Proposal::new(sugar::choice(10, 20))).unwrap();
        let s = arb.state();
        assert!(s.committed.contains(&10));
        assert!(!s.option_space.contains(&10));
        assert!(s.refused.iter().any(|(_, a, _)| *a == Some(20)));
        // The rejected branch's symbol is NOT committed, and remains
        // structurally available (Refuse does not consume Omega).
        assert!(s.option_space.contains(&20));
    }

    #[test]
    fn merge_is_collapse_of_a_bind() {
        let mut arb = Arbiter::new(omega(&[1, 2, 3, 4]), ["merge_quotient"]);
        arb.submit(Proposal::new(sugar::merge(1, 2, "merge_quotient")))
            .unwrap();
        let mut classes = collapse::collapse_quotient(arb.history_ref());
        assert!(classes.same_class(1, 2));
        assert!(!classes.same_class(1, 3));
        // Merge never needed to be a fifth event kind.
        assert!(arb
            .history_ref()
            .events_of_kind(EventKind::Bind)
            .any(|e| e.a == Some(1) && e.b == Some(2)));
    }

    #[test]
    fn set_meta_is_invisible_to_the_ordinary_quotient_rule() {
        let mut arb = Arbiter::new(omega(&[1, 100]), []);
        arb.submit(Proposal::new(vec![sugar::set_meta(1, 100)]))
            .unwrap();
        let mut classes = collapse::collapse_quotient(arb.history_ref());
        // A metadata bind must NOT be treated as an ordinary Merge/Link.
        assert!(!classes.same_class(1, 100));
        // But it IS visible to the dedicated metadata rule.
        let meta = collapse::collapse_meta(arb.history_ref());
        assert!(meta.contains_key(&1));
    }

    // ---- Kernel-level invariants --------------------------------------

    #[test]
    fn deterministic_replay() {
        let mut arb = Arbiter::new(omega(&[1, 2, 3]), ["identity"]);
        arb.submit(Proposal::new(vec![
            Event::pop(1),
            Event::bind(1, 2, "adjacent"),
            Event::collapse("identity"),
        ]))
        .unwrap();

        let s1 = arb.state();
        let s2 = arb.state(); // replay again from the same H
        assert_eq!(s1, s2);
    }

    #[test]
    fn irreversibility_history_only_grows() {
        let mut arb = Arbiter::new(omega(&[1, 2]), []);
        assert_eq!(arb.len(), 0);
        arb.submit(Proposal::new(vec![Event::pop(1)])).unwrap();
        assert_eq!(arb.len(), 1);
        arb.submit(Proposal::new(vec![Event::bind(1, 2, "x")]))
            .unwrap();
        assert_eq!(arb.len(), 2);
        // No API in this crate can shrink `len()` — there is no remove/undo.
    }

    #[test]
    fn conservation_of_possibility() {
        let omega_0 = omega(&[1, 2, 3, 4]);
        let mut arb = Arbiter::new(omega_0.clone(), ["identity"]);
        let pi_0 = possibility_functional(&arb.state());
        assert_eq!(pi_0, omega_0.len());

        arb.submit(Proposal::new(vec![Event::pop(1)])).unwrap();
        assert_eq!(possibility_functional(&arb.state()), pi_0);

        arb.submit(Proposal::new(vec![Event::refuse(2, "excluded")]))
            .unwrap();
        assert_eq!(possibility_functional(&arb.state()), pi_0);

        arb.submit(Proposal::new(vec![Event::bind(3, 4, "rel")]))
            .unwrap();
        assert_eq!(possibility_functional(&arb.state()), pi_0);

        arb.submit(Proposal::new(vec![Event::collapse("identity")]))
            .unwrap();
        assert_eq!(possibility_functional(&arb.state()), pi_0);
    }

    // ---- Overlay / preview-commit --------------------------------------

    #[test]
    fn preview_does_not_mutate_history() {
        let mut arb = Arbiter::new(omega(&[1, 2]), []);
        arb.submit(Proposal::new(vec![Event::pop(1), Event::bind(1, 2, "adjacent")]))
            .unwrap();
        let len_before = arb.len();

        let overlay_events = vec![sugar::unlink(1, 2)];
        let om = OverlayManager::new(&mut arb);
        let ov = om.create(Proposal::new(overlay_events));
        let speculative = om.preview(&ov);

        assert!(speculative
            .refused
            .iter()
            .any(|(_, _, r)| r == "relation withdrawn"));
        assert_eq!(arb.len(), len_before); // H itself is unchanged by preview
        assert!(!arb
            .state()
            .refused
            .iter()
            .any(|(_, _, r)| r == "relation withdrawn"));
    }

    #[test]
    fn stale_overlay_commit_is_rejected() {
        let mut arb = Arbiter::new(omega(&[1, 2, 3]), []);
        let ov = OverlayManager::new(&mut arb).create(Proposal::new(vec![Event::pop(1)]));

        // H moves after the overlay was created but before it is committed.
        arb.submit(Proposal::new(vec![Event::pop(2)])).unwrap();

        let err = OverlayManager::new(&mut arb).commit(ov).unwrap_err();
        assert_eq!(err, ArbiterError::StaleOverlay);
    }

    #[test]
    fn commit_makes_the_overlay_authoritative() {
        let mut arb = Arbiter::new(omega(&[1, 2]), []);
        arb.submit(Proposal::new(vec![Event::bind(1, 2, "adjacent")]))
            .unwrap();

        let ov = OverlayManager::new(&mut arb).create(Proposal::new(vec![sugar::unlink(1, 2)]));
        OverlayManager::new(&mut arb).commit(ov).unwrap();

        assert!(arb
            .state()
            .refused
            .iter()
            .any(|(_, _, r)| r == "relation withdrawn"));
    }

    // ---- Observation non-interference ----------------------------------

    #[test]
    fn observation_cannot_influence_later_acceptance() {
        // Two independent runs differing only in what a Collapse would
        // observe (achieved here by binding different pairs before
        // collapsing) must accept or reject an identical subsequent Pop
        // in exactly the same way — the arbiter's decision function has
        // no parameter through which any observed value could flow.
        let mut arb_a = Arbiter::new(omega(&[1, 2, 3]), ["identity"]);
        arb_a
            .submit(Proposal::new(vec![Event::bind(1, 2, "x"), Event::collapse("identity")]))
            .unwrap();

        let mut arb_b = Arbiter::new(omega(&[1, 2, 3]), ["identity"]);
        arb_b
            .submit(Proposal::new(vec![Event::bind(1, 3, "y"), Event::collapse("identity")]))
            .unwrap();

        let result_a = arb_a.submit(Proposal::new(vec![Event::pop(2)]));
        let result_b = arb_b.submit(Proposal::new(vec![Event::pop(2)]));
        assert_eq!(result_a.is_ok(), result_b.is_ok());
    }

    #[test]
    fn canonical_history_wire_round_trip() {
        let mut arb = Arbiter::new(omega(&[3, 1, 2]), ["identity"]);
        arb.submit(Proposal::new(vec![
            Event::pop(1),
            Event::bind(1, 2, "adjacent"),
            Event::collapse("identity"),
        ])).unwrap();
        let bytes = wire::encode_history([3, 1, 2], ["identity"], arb.history_ref()).unwrap();
        assert_eq!(wire::fnv1a64(&bytes), "bf4988c6e7a3c379");
        let decoded = wire::decode_history(&bytes).unwrap();
        assert_eq!(decoded.history.replay(&decoded.initial_option_space), arb.state());
        let mut trailing = bytes;
        trailing.push(0);
        assert!(wire::decode_history(&trailing).is_err());
    }
}
