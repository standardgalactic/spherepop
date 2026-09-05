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
	out.Refused = append([]Refusal(nil), s.Refused...)
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
		s.Refused = append(s.Refused, Refusal{e.Pos, e.A, e.B, r})
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
	return append([]Event(nil), h.events...)
}

func (h History) EventsOfKind(kind EventKind) []Event {
	out := make([]Event, 0)
	for _, e := range h.events {
		if e.Kind == kind {
			out = append(out, e)
		}
	}
	return out
}

func (h *History) append(e Event) { h.events = append(h.events, e) }

// Meld is structural composition of histories, not a fifth primitive.
func (h History) Meld(other History) History {
	out := NewHistory()
	out.events = append(out.events, h.events...)
	out.events = append(out.events, other.events...)
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
