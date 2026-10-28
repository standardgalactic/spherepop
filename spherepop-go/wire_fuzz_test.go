package spherepop

import (
	"bytes"
	"testing"
)

// FuzzWireDecode checks that DecodeHistory never panics on arbitrary input and
// that every envelope it accepts is canonical: re-encoding the decoded world
// reproduces the input bytes exactly.
func FuzzWireDecode(f *testing.F) {
	seed := History{events: []Event{
		PopEvent(1),
		RefuseEvent(2, "reason"),
		RefuseBindEvent(1, 2, "pair"),
		BindEvent(1, 2, "tag"),
		CollapseEvent("identity"),
	}}
	if data, err := EncodeHistory([]ObjectID{1, 2}, []RuleID{"identity"}, seed); err == nil {
		f.Add(data)
	}
	f.Add(wireMagic[:])
	f.Add([]byte{})
	f.Fuzz(func(t *testing.T, data []byte) {
		world, err := DecodeHistory(data)
		if err != nil {
			return
		}
		again, err := EncodeHistory(world.InitialOptionSpace, world.CertifiedRules, world.History)
		if err != nil {
			t.Fatalf("decoded world failed to re-encode: %v", err)
		}
		if !bytes.Equal(data, again) {
			t.Fatal("accepted envelope is not canonical: re-encoding changed bytes")
		}
	})
}
