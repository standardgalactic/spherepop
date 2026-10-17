package spherepop

import (
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
