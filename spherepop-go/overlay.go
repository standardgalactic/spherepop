package spherepop

type Overlay struct {
	baseLen int
	pending Proposal
}

type OverlayManager struct{ Arbiter *Arbiter }

func NewOverlayManager(a *Arbiter) *OverlayManager {
	return &OverlayManager{Arbiter: a}
}

func (m *OverlayManager) Create(p Proposal) Overlay {
	return Overlay{baseLen: m.Arbiter.Len(), pending: p}
}

// Preview is non-authoritative and does not validate or mutate history.
func (m *OverlayManager) Preview(o Overlay) State {
	h := m.Arbiter.History()
	for _, e := range o.pending.Events {
		e.Pos = LogPos(h.Len())
		h.append(e)
	}
	return h.Replay(m.Arbiter.omega0)
}

func (m *OverlayManager) Commit(o Overlay) ([]LogPos, error) {
	if o.baseLen != m.Arbiter.Len() {
		return nil, &ArbiterError{StaleOverlay, ""}
	}
	return m.Arbiter.Submit(o.pending)
}
