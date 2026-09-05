package spherepop

import "fmt"

type ErrorCode string

const (
	Malformed               ErrorCode = "Malformed"
	PopOutsideOptionSpace   ErrorCode = "PopOutsideOptionSpace"
	UncertifiedCollapseRule ErrorCode = "UncertifiedCollapseRule"
	RefuseWithoutReason     ErrorCode = "RefuseWithoutReason"
	StaleOverlay            ErrorCode = "StaleOverlay"
)

type ArbiterError struct {
	Code   ErrorCode
	Detail string
}

func (e *ArbiterError) Error() string {
	if e.Detail == "" {
		return string(e.Code)
	}
	return fmt.Sprintf("%s(%s)", e.Code, e.Detail)
}

type Proposal struct{ Events []Event }

func NewProposal(events ...Event) Proposal {
	return Proposal{Events: append([]Event(nil), events...)}
}

type Arbiter struct {
	history History
	rules   map[RuleID]struct{}
	omega0  []ObjectID
}

func NewArbiter(omega []ObjectID, certifiedRules []RuleID) *Arbiter {
	a := &Arbiter{
		history: NewHistory(),
		rules:   make(map[RuleID]struct{}, len(certifiedRules)),
		omega0:  append([]ObjectID(nil), omega...),
	}
	for _, id := range certifiedRules {
		a.rules[id] = struct{}{}
	}
	return a
}

func (a *Arbiter) Len() int         { return a.history.Len() }
func (a *Arbiter) History() History { return History{events: a.history.Events()} }
func (a *Arbiter) State() State     { return a.history.Replay(a.omega0) }

// Submit atomically validates the complete proposal against the state after
// each preceding proposed event, then appends every event or none of them.
func (a *Arbiter) Submit(p Proposal) ([]LogPos, error) {
	if err := a.validate(p.Events); err != nil {
		return nil, err
	}
	positions := make([]LogPos, 0, len(p.Events))
	for _, event := range p.Events {
		event.Pos = LogPos(a.history.Len())
		a.history.append(event)
		positions = append(positions, event.Pos)
	}
	return positions, nil
}

func (a *Arbiter) validate(events []Event) error {
	hypothetical := a.State()
	for _, event := range events {
		switch event.Kind {
		case Pop:
			if event.A == nil {
				return &ArbiterError{Malformed, "Pop missing a"}
			}
			if _, ok := hypothetical.OptionSpace[*event.A]; !ok {
				return &ArbiterError{PopOutsideOptionSpace, fmt.Sprint(*event.A)}
			}
		case Refuse:
			if event.Reason == nil || *event.Reason == "" {
				return &ArbiterError{RefuseWithoutReason, ""}
			}
		case Bind:
			if event.A == nil || event.B == nil {
				return &ArbiterError{Malformed, "Bind missing a/b"}
			}
		case Collapse:
			if event.Rule == nil {
				return &ArbiterError{Malformed, "Collapse missing rule"}
			}
			if _, ok := a.rules[*event.Rule]; !ok {
				return &ArbiterError{UncertifiedCollapseRule, string(*event.Rule)}
			}
		default:
			return &ArbiterError{Malformed, "unknown event kind"}
		}
		Apply(&hypothetical, event)
	}
	return nil
}
