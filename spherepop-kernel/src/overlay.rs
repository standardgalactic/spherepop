//! Overlay manager: speculative proposals previewed against H without
//! ever mutating it, and committed only through an explicit, separate
//! call that routes through `Arbiter::submit` like any other proposal.
//!
//! There is deliberately no `auto_commit` method and no default argument
//! making `commit` implicit in `create` or `preview` — "no utility may
//! auto-commit by default" is the absence of a code path here, not a
//! documented convention.

use crate::arbiter::{Arbiter, ArbiterError, Proposal};
use crate::event::LogPos;
use crate::history::State;

pub struct Overlay {
    base_len: usize, // H.len() at the time the overlay was created
    pending: Proposal,
}

pub struct OverlayManager<'a> {
    pub arbiter: &'a mut Arbiter,
}

impl<'a> OverlayManager<'a> {
    pub fn new(arbiter: &'a mut Arbiter) -> Self {
        OverlayManager { arbiter }
    }

    pub fn create(&self, pending: Proposal) -> Overlay {
        Overlay {
            base_len: self.arbiter.len(),
            pending,
        }
    }

    /// Non-authoritative: replays H + overlay without touching H.
    pub fn preview(&self, o: &Overlay) -> State {
        let mut speculative = self.arbiter.history_clone();
        for e in o.pending.events.clone() {
            speculative.push(e);
        }
        speculative.replay(self.arbiter.omega_0_ref())
    }

    /// The only call that can make an overlay authoritative.
    pub fn commit(&mut self, o: Overlay) -> Result<Vec<LogPos>, ArbiterError> {
        if o.base_len != self.arbiter.len() {
            return Err(ArbiterError::StaleOverlay); // H moved since preview
        }
        self.arbiter.submit(o.pending)
    }
}
