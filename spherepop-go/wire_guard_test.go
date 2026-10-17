package spherepop

import (
	"bytes"
	"encoding/binary"
	"testing"
)

func TestWireRejectsInvalidUTF8(t *testing.T) {
	bad := string([]byte{0xff})
	cases := []struct {
		name    string
		rules   []RuleID
		history History
	}{
		{"certified rule", []RuleID{RuleID(bad)}, NewHistory()},
		{"refusal reason", nil, History{events: []Event{RefuseEvent(1, bad)}}},
		{"bind tag", nil, History{events: []Event{BindEvent(1, 2, bad)}}},
		{"collapse rule", nil, History{events: []Event{CollapseEvent(RuleID(bad))}}},
	}
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			if _, err := EncodeHistory(nil, tc.rules, tc.history); err == nil {
				t.Fatal("encoder accepted invalid UTF-8 that DecodeHistory must reject")
			}
		})
	}
}

func TestWireRejectsMissingEventFields(t *testing.T) {
	cases := []struct {
		name  string
		event Event
	}{
		{"pop a", Event{Kind: Pop}},
		{"refuse a", Event{Kind: Refuse, Reason: text("reason")}},
		{"refuse reason", Event{Kind: Refuse, A: object(1)}},
		{"bind a", Event{Kind: Bind, B: object(2), Tag: text("tag")}},
		{"bind b", Event{Kind: Bind, A: object(1), Tag: text("tag")}},
		{"bind tag", Event{Kind: Bind, A: object(1), B: object(2)}},
		{"collapse rule", Event{Kind: Collapse}},
	}
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			_, err := EncodeHistory(nil, nil, History{events: []Event{tc.event}})
			if err == nil {
				t.Fatal("encoder accepted malformed primitive instead of returning an error")
			}
		})
	}
}

func TestWireRejectsImpossibleDeclaredCounts(t *testing.T) {
	tests := []struct {
		name        string
		headerBytes int
		countOffset int
	}{
		{"options", 12, 8},
		{"rules", 16, 12},
		{"events", 20, 16},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			data := make([]byte, tc.headerBytes)
			copy(data, wireMagic[:])
			binary.BigEndian.PutUint32(data[tc.countOffset:], ^uint32(0))
			if _, err := DecodeHistory(data); err == nil {
				t.Fatal("decoder accepted impossible declared length")
			}
		})
	}
}

func TestWireRoundTripUnicode(t *testing.T) {
	h := History{events: []Event{
		RefuseEvent(3, "Gründe"),
		BindEvent(3, 4, "邻接"),
		CollapseEvent("égalité"),
	}}
	data, err := EncodeHistory([]ObjectID{4, 3}, []RuleID{"égalité"}, h)
	if err != nil {
		t.Fatal(err)
	}
	got, err := DecodeHistory(data)
	if err != nil {
		t.Fatal(err)
	}
	reencoded, err := EncodeHistory(got.InitialOptionSpace, got.CertifiedRules, got.History)
	if err != nil {
		t.Fatal(err)
	}
	if !bytes.Equal(data, reencoded) {
		t.Fatal("canonical Unicode round-trip changed bytes")
	}
}
