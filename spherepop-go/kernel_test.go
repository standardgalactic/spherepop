package spherepop

import (
	"reflect"
	"testing"
)

func TestPrimitiveSemantics(t *testing.T) {
	a := NewArbiter([]ObjectID{1, 2, 3}, []RuleID{"quotient"})
	if _, err := a.Submit(NewProposal(
		PopEvent(1),
		RefuseEvent(2, "not now"),
		BindEvent(2, 3, "adjacent"),
		CollapseEvent("quotient"),
	)); err != nil {
		t.Fatal(err)
	}
	s := a.State()
	if _, ok := s.OptionSpace[1]; ok {
		t.Fatal("Pop did not remove 1 from Omega")
	}
	if _, ok := s.OptionSpace[2]; !ok {
		t.Fatal("Refuse incorrectly removed 2 from Omega")
	}
	if _, ok := s.Bound[Binding{2, 3, "adjacent"}]; !ok {
		t.Fatal("Bind was not replayed")
	}
	if len(s.Observed) != 1 || s.Observed[0].Rule != "quotient" {
		t.Fatal("Collapse invocation was not recorded")
	}
}

func TestAtomicProposal(t *testing.T) {
	a := NewArbiter([]ObjectID{1}, nil)
	_, err := a.Submit(NewProposal(PopEvent(1), PopEvent(1)))
	if err == nil || a.Len() != 0 {
		t.Fatalf("proposal must be rejected atomically: len=%d err=%v", a.Len(), err)
	}
}

func TestMeldAndQuotient(t *testing.T) {
	a := NewArbiter([]ObjectID{1, 2}, nil)
	b := NewArbiter([]ObjectID{3, 4}, nil)
	_, _ = a.Submit(NewProposal(BindEvent(1, 2, "x")))
	_, _ = b.Submit(NewProposal(BindEvent(3, 4, "y")))
	h := a.History().Meld(b.History())
	if h.Len() != 2 {
		t.Fatalf("meld len=%d", h.Len())
	}
	q := CollapseQuotient(h)
	if !q.SameClass(1, 2) || !q.SameClass(3, 4) || q.SameClass(1, 3) {
		t.Fatal("quotient classes incorrect")
	}
}

func TestOverlayStaleness(t *testing.T) {
	a := NewArbiter([]ObjectID{1, 2}, nil)
	m := NewOverlayManager(a)
	o := m.Create(NewProposal(PopEvent(1)))
	preview := m.Preview(o)
	if _, ok := preview.Committed[1]; !ok || a.Len() != 0 {
		t.Fatal("preview must be visible but non-authoritative")
	}
	_, _ = a.Submit(NewProposal(PopEvent(2)))
	if _, err := m.Commit(o); err == nil {
		t.Fatal("stale overlay was committed")
	}
}

func TestCanonicalHistoryWireRoundTrip(t *testing.T) {
	a := NewArbiter([]ObjectID{3, 1, 2}, []RuleID{"identity"})
	_, err := a.Submit(NewProposal(PopEvent(1), BindEvent(1, 2, "adjacent"), CollapseEvent("identity")))
	if err != nil {
		t.Fatal(err)
	}
	wire, err := EncodeHistory([]ObjectID{3, 1, 2}, []RuleID{"identity"}, a.History())
	if err != nil {
		t.Fatal(err)
	}
	if got := FNV1a64(wire); got != "bf4988c6e7a3c379" {
		t.Fatalf("digest=%s", got)
	}
	decoded, err := DecodeHistory(wire)
	if err != nil {
		t.Fatal(err)
	}
	if !reflect.DeepEqual(decoded.History.Replay(decoded.InitialOptionSpace), a.State()) {
		t.Fatal("round-trip replay changed state")
	}
	if _, err := DecodeHistory(append(wire, 0)); err == nil {
		t.Fatal("trailing byte was accepted")
	}
}
