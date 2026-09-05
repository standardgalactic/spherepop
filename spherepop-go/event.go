package spherepop

// ObjectID names an object in the option space. LogPos is a zero-based
// position in authoritative history, and RuleID names a certified Collapse rule.
type ObjectID uint64
type LogPos uint64
type RuleID string

// EventKind is the closed primitive alphabet. Derived operations never add a
// value here.
type EventKind uint8

const (
	Pop EventKind = iota
	Refuse
	Bind
	Collapse
)

func (k EventKind) String() string {
	switch k {
	case Pop:
		return "Pop"
	case Refuse:
		return "Refuse"
	case Bind:
		return "Bind"
	case Collapse:
		return "Collapse"
	default:
		return "Unknown"
	}
}

// Event uses pointers for fields that are absent for a particular primitive.
// This preserves the specification's distinction between zero and not present.
type Event struct {
	Kind   EventKind
	Pos    LogPos
	A      *ObjectID
	B      *ObjectID
	Tag    *string
	Reason *string
	Rule   *RuleID
}

func object(v ObjectID) *ObjectID { return &v }
func text(v string) *string       { return &v }
func rule(v RuleID) *RuleID       { return &v }

func PopEvent(x ObjectID) Event {
	return Event{Kind: Pop, A: object(x)}
}

func RefuseEvent(x ObjectID, reason string) Event {
	return Event{Kind: Refuse, A: object(x), Reason: text(reason)}
}

func RefuseBindEvent(a, b ObjectID, reason string) Event {
	return Event{Kind: Refuse, A: object(a), B: object(b), Reason: text(reason)}
}

func BindEvent(a, b ObjectID, tag string) Event {
	return Event{Kind: Bind, A: object(a), B: object(b), Tag: text(tag)}
}

func CollapseEvent(id RuleID) Event {
	return Event{Kind: Collapse, Rule: rule(id)}
}
