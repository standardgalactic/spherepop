package spherepop

import "testing"

func TestHistoryIsImmutableAcrossPointerBoundaries(t *testing.T) {
	a := NewArbiter([]ObjectID{1, 2}, []RuleID{"identity"})
	input := PopEvent(1)
	proposal := NewProposal(input)
	*input.A = 2
	if _, err := a.Submit(proposal); err != nil {
		t.Fatal(err)
	}
	if _, ok := a.State().Committed[1]; !ok {
		t.Fatal("caller mutated a proposal through an original event pointer")
	}
	events := a.History().Events()
	*events[0].A = 2
	if _, ok := a.State().Committed[1]; !ok {
		t.Fatal("caller mutated authoritative history through Events()")
	}
	if _, ok := a.State().Committed[2]; ok {
		t.Fatal("history changed after caller modified exported event pointer")
	}
}

func TestHistoryAppendDetachesProposalEventFields(t *testing.T) {
	a := NewArbiter([]ObjectID{1, 2}, nil)
	p := NewProposal(BindEvent(1, 2, "edge"))
	if _, err := a.Submit(p); err != nil {
		t.Fatal(err)
	}
	*p.Events[0].Tag = "changed"
	if _, ok := a.State().Bound[Binding{1, 2, "edge"}]; !ok {
		t.Fatal("mutable proposal tag changed committed history")
	}
}

func TestReplayProjectionCannotMutateHistory(t *testing.T) {
	a := NewArbiter([]ObjectID{1}, nil)
	if _, err := a.Submit(NewProposal(RefuseEvent(1, "original"))); err != nil {
		t.Fatal(err)
	}
	projected := a.State()
	*projected.Refused[0].Target = 999
	if got := *a.State().Refused[0].Target; got != 1 {
		t.Fatalf("state projection mutated authoritative history: got %d", got)
	}
}

func TestOverlayCreationSnapshotsProposalFields(t *testing.T) {
	a := NewArbiter([]ObjectID{1, 2}, nil)
	manager := NewOverlayManager(a)
	proposal := NewProposal(PopEvent(1))
	overlay := manager.Create(proposal)
	*proposal.Events[0].A = 2

	preview := manager.Preview(overlay)
	if _, ok := preview.Committed[1]; !ok {
		t.Fatal("overlay preview changed after its source proposal was modified")
	}
	if _, ok := preview.Committed[2]; ok {
		t.Fatal("overlay preview used modified caller-owned event")
	}
	positions, err := manager.Commit(overlay)
	if err != nil {
		t.Fatal(err)
	}
	if len(positions) != 1 || positions[0] != 0 {
		t.Fatalf("unexpected commit positions: %v", positions)
	}
	if _, ok := a.State().Committed[1]; !ok {
		t.Fatal("overlay commit used modified caller-owned event")
	}
}
