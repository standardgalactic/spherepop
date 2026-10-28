package spherepop

// Binding is a recorded dependency. Relatedness never implies identity.
type Binding struct {
	A   ObjectID
	B   ObjectID
	Tag string
}

type Refusal struct {
	Pos    LogPos
	Target *ObjectID
	Other  *ObjectID
	Reason string
}

type Observation struct {
	Pos  LogPos
	Rule RuleID
}

// State is a deterministic replay product, never the authoritative source.
type State struct {
	OptionSpace map[ObjectID]struct{}
	Committed   map[ObjectID]struct{}
	Bound       map[Binding]struct{}
	Refused     []Refusal
	Observed    []Observation
}

func NewState(omega []ObjectID) State {
	s := State{
		OptionSpace: make(map[ObjectID]struct{}, len(omega)),
		Committed:   make(map[ObjectID]struct{}),
		Bound:       make(map[Binding]struct{}),
	}
	for _, id := range omega {
		s.OptionSpace[id] = struct{}{}
	}
	return s
}

func (s State) Clone() State {
	out := NewState(nil)
	for id := range s.OptionSpace {
		out.OptionSpace[id] = struct{}{}
	}
	for id := range s.Committed {
		out.Committed[id] = struct{}{}
	}
	for b := range s.Bound {
		out.Bound[b] = struct{}{}
	}
	for _, refused := range s.Refused {
		if refused.Target != nil {
			v := *refused.Target
			refused.Target = &v
		}
		if refused.Other != nil {
			v := *refused.Other
			refused.Other = &v
		}
		out.Refused = append(out.Refused, refused)
	}
	out.Observed = append([]Observation(nil), s.Observed...)
	return out
}

func Apply(s *State, e Event) {
	switch e.Kind {
	case Pop:
		delete(s.OptionSpace, *e.A)
		s.Committed[*e.A] = struct{}{}
	case Refuse:
		r := ""
		if e.Reason != nil {
			r = *e.Reason
		}
		copied := cloneEvent(e)
		s.Refused = append(s.Refused, Refusal{e.Pos, copied.A, copied.B, r})
	case Bind:
		t := ""
		if e.Tag != nil {
			t = *e.Tag
		}
		s.Bound[Binding{*e.A, *e.B, t}] = struct{}{}
	case Collapse:
		s.Observed = append(s.Observed, Observation{e.Pos, *e.Rule})
	}
}

// History is append-only. Events returns a defensive copy.
type History struct{ events []Event }

func NewHistory() History  { return History{} }
func (h History) Len() int { return len(h.events) }

func (h History) Events() []Event {
	out := make([]Event, len(h.events))
	for i, e := range h.events {
		out[i] = cloneEvent(e)
	}
	return out
}

func cloneEvent(e Event) Event {
	if e.A != nil {
		a := *e.A
		e.A = &a
	}
	if e.B != nil {
		b := *e.B
		e.B = &b
	}
	if e.Tag != nil {
		tag := *e.Tag
		e.Tag = &tag
	}
	if e.Reason != nil {
		reason := *e.Reason
		e.Reason = &reason
	}
	if e.Rule != nil {
		rule := *e.Rule
		e.Rule = &rule
	}
	return e
}

func (h History) EventsOfKind(kind EventKind) []Event {
	out := make([]Event, 0)
	for _, e := range h.events {
		if e.Kind == kind {
			out = append(out, cloneEvent(e))
		}
	}
	return out
}

func (h *History) append(e Event) { h.events = append(h.events, cloneEvent(e)) }

// Meld is structural composition of histories, not a fifth primitive.
func (h History) Meld(other History) History {
	out := NewHistory()
	out.events = append(out.events, h.Events()...)
	out.events = append(out.events, other.Events()...)
	for i := range out.events {
		out.events[i].Pos = LogPos(i)
	}
	return out
}

func (h History) Replay(omega []ObjectID) State {
	s := NewState(omega)
	for _, e := range h.events {
		Apply(&s, e)
	}
	return s
}

// PossibilityFunctional is |Omega| + committed Pop count. All non-Pop
// primitive weights are zero in SPEC v1.
func PossibilityFunctional(s State) int {
	return len(s.OptionSpace) + len(s.Committed)
}
