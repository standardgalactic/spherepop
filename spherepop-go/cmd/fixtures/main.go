package main

import (
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"reflect"
	"sort"
	"strings"

	sp "github.com/standardgalactic/spherepop/spherepop-go"
)

type fixtureEvent struct {
	Op           string `json:"op"`
	A            uint64 `json:"a"`
	B            uint64 `json:"b"`
	Taken        uint64 `json:"taken"`
	Rejected     uint64 `json:"rejected"`
	Object       uint64 `json:"object"`
	Key          uint64 `json:"key"`
	Tag          string `json:"tag"`
	Reason       string `json:"reason"`
	Rule         string `json:"rule"`
	ExpectReject bool   `json:"expect_reject"`
	ExpectError  string `json:"expect_error"`
}

type subHistory struct {
	InitialOptionSpace []uint64       `json:"initial_option_space"`
	CertifiedRules     []string       `json:"certified_rules"`
	Events             []fixtureEvent `json:"events"`
}

type expectation struct {
	OptionSpace                       *[]uint64 `json:"option_space"`
	Committed                         *[]uint64 `json:"committed"`
	Bound                             *[][]any  `json:"bound"`
	RefusedCount                      *int      `json:"refused_count"`
	ObservedRules                     *[]string `json:"observed_rules"`
	HistoryLen                        *int      `json:"history_len"`
	QuotientSameClass                 *[][]any  `json:"quotient_same_class"`
	QuotientHonoringRefusalsSameClass *[][]any  `json:"quotient_honoring_refusals_same_class"`
	MetaKeys                          *[]uint64 `json:"meta_keys"`
	DeterministicReplay               bool      `json:"deterministic_replay"`
	CanonicalHistoryFNV1a64           string    `json:"canonical_history_fnv1a64"`
}

type fixture struct {
	Name                   string         `json:"name"`
	Manual                 bool           `json:"manual"`
	Invariant              string         `json:"invariant"`
	Explanation            string         `json:"explanation"`
	InitialOptionSpace     []uint64       `json:"initial_option_space"`
	CertifiedRules         []string       `json:"certified_rules"`
	Events                 []fixtureEvent `json:"events"`
	Expect                 *expectation   `json:"expect"`
	HistoryA               *subHistory    `json:"history_a"`
	HistoryB               *subHistory    `json:"history_b"`
	ExpectMeldedHistoryLen *int           `json:"expect_melded_history_len"`
}

func ids(in []uint64) []sp.ObjectID {
	out := make([]sp.ObjectID, len(in))
	for i, value := range in {
		out[i] = sp.ObjectID(value)
	}
	return out
}

func rules(in []string) []sp.RuleID {
	out := make([]sp.RuleID, len(in))
	for i, value := range in {
		out[i] = sp.RuleID(value)
	}
	return out
}

func eventsFor(e fixtureEvent) ([]sp.Event, error) {
	switch e.Op {
	case "pop":
		return []sp.Event{sp.PopEvent(sp.ObjectID(e.A))}, nil
	case "refuse":
		return []sp.Event{sp.RefuseEvent(sp.ObjectID(e.A), e.Reason)}, nil
	case "refuse_bind":
		return []sp.Event{sp.RefuseBindEvent(sp.ObjectID(e.A), sp.ObjectID(e.B), e.Reason)}, nil
	case "bind":
		return []sp.Event{sp.BindEvent(sp.ObjectID(e.A), sp.ObjectID(e.B), e.Tag)}, nil
	case "collapse":
		return []sp.Event{sp.CollapseEvent(sp.RuleID(e.Rule))}, nil
	case "link":
		return []sp.Event{sp.Link(sp.ObjectID(e.A), sp.ObjectID(e.B), e.Tag)}, nil
	case "unlink":
		return []sp.Event{sp.Unlink(sp.ObjectID(e.A), sp.ObjectID(e.B))}, nil
	case "choice":
		return sp.Choice(sp.ObjectID(e.Taken), sp.ObjectID(e.Rejected)), nil
	case "merge":
		return sp.Merge(sp.ObjectID(e.A), sp.ObjectID(e.B), sp.RuleID(e.Rule)), nil
	case "set_meta":
		return []sp.Event{sp.SetMeta(sp.ObjectID(e.Object), sp.ObjectID(e.Key))}, nil
	default:
		return nil, fmt.Errorf("unknown fixture op %q", e.Op)
	}
}

func execute(a *sp.Arbiter, fixtureEvents []fixtureEvent) []string {
	var failures []string
	for _, input := range fixtureEvents {
		events, err := eventsFor(input)
		if err != nil {
			failures = append(failures, err.Error())
			continue
		}
		before := a.Len()
		_, err = a.Submit(sp.NewProposal(events...))
		if input.ExpectReject {
			if err == nil {
				failures = append(failures, fmt.Sprintf("%s was accepted but rejection was expected", input.Op))
			} else if input.ExpectError != "" && !strings.HasPrefix(err.Error(), input.ExpectError) {
				failures = append(failures, fmt.Sprintf("expected error %s, got %v", input.ExpectError, err))
			}
			if a.Len() != before {
				failures = append(failures, "rejected proposal mutated history")
			}
		} else if err != nil {
			failures = append(failures, fmt.Sprintf("%s rejected unexpectedly: %v", input.Op, err))
		}
	}
	return failures
}

func sortedSet(values map[sp.ObjectID]struct{}) []uint64 {
	out := make([]uint64, 0, len(values))
	for value := range values {
		out = append(out, uint64(value))
	}
	sort.Slice(out, func(i, j int) bool { return out[i] < out[j] })
	return out
}

func checkTriples(rows [][]any, check func(sp.ObjectID, sp.ObjectID) bool, name string) []string {
	var failures []string
	for _, row := range rows {
		if len(row) != 3 {
			failures = append(failures, name+": expected triple")
			continue
		}
		a, aok := row[0].(float64)
		b, bok := row[1].(float64)
		expected, eok := row[2].(bool)
		if !aok || !bok || !eok {
			failures = append(failures, name+": malformed triple")
			continue
		}
		actual := check(sp.ObjectID(a), sp.ObjectID(b))
		if actual != expected {
			failures = append(failures, fmt.Sprintf("%s(%d,%d): expected %v, got %v", name, uint64(a), uint64(b), expected, actual))
		}
	}
	return failures
}

func checkExpected(a *sp.Arbiter, initial []sp.ObjectID, certified []sp.RuleID, expect *expectation) []string {
	if expect == nil {
		return nil
	}
	state := a.State()
	var failures []string
	if expect.OptionSpace != nil && !reflect.DeepEqual(sortedSet(state.OptionSpace), *expect.OptionSpace) {
		failures = append(failures, fmt.Sprintf("option_space: expected %v, got %v", *expect.OptionSpace, sortedSet(state.OptionSpace)))
	}
	if expect.Committed != nil && !reflect.DeepEqual(sortedSet(state.Committed), *expect.Committed) {
		failures = append(failures, fmt.Sprintf("committed: expected %v, got %v", *expect.Committed, sortedSet(state.Committed)))
	}
	if expect.RefusedCount != nil && len(state.Refused) != *expect.RefusedCount {
		failures = append(failures, fmt.Sprintf("refused_count: expected %d, got %d", *expect.RefusedCount, len(state.Refused)))
	}
	if expect.HistoryLen != nil && a.Len() != *expect.HistoryLen {
		failures = append(failures, fmt.Sprintf("history_len: expected %d, got %d", *expect.HistoryLen, a.Len()))
	}
	if expect.ObservedRules != nil {
		actual := make([]string, len(state.Observed))
		for i, observation := range state.Observed {
			actual[i] = string(observation.Rule)
		}
		if !reflect.DeepEqual(actual, *expect.ObservedRules) {
			failures = append(failures, fmt.Sprintf("observed_rules: expected %v, got %v", *expect.ObservedRules, actual))
		}
	}
	if expect.Bound != nil {
		for _, row := range *expect.Bound {
			if len(row) != 3 {
				failures = append(failures, "bound: malformed triple")
				continue
			}
			a, aok := row[0].(float64)
			b, bok := row[1].(float64)
			tag, tok := row[2].(string)
			if !aok || !bok || !tok {
				failures = append(failures, "bound: malformed values")
				continue
			}
			if _, ok := state.Bound[sp.Binding{A: sp.ObjectID(a), B: sp.ObjectID(b), Tag: tag}]; !ok {
				failures = append(failures, fmt.Sprintf("bound: missing (%d,%d,%q)", uint64(a), uint64(b), tag))
			}
		}
	}
	history := a.History()
	if expect.QuotientSameClass != nil {
		q := sp.CollapseQuotient(history)
		failures = append(failures, checkTriples(*expect.QuotientSameClass, q.SameClass, "quotient_same_class")...)
	}
	if expect.QuotientHonoringRefusalsSameClass != nil {
		q := sp.CollapseQuotientHonoringRefusals(history)
		failures = append(failures, checkTriples(*expect.QuotientHonoringRefusalsSameClass, q.SameClass, "quotient_honoring_refusals_same_class")...)
	}
	if expect.MetaKeys != nil {
		meta := sp.CollapseMeta(history)
		for _, key := range *expect.MetaKeys {
			if _, ok := meta[sp.ObjectID(key)]; !ok {
				failures = append(failures, fmt.Sprintf("meta_keys: missing %d", key))
			}
		}
	}
	if expect.DeterministicReplay {
		if !reflect.DeepEqual(history.Replay(initial), history.Replay(initial)) {
			failures = append(failures, "deterministic replay disagreed")
		}
	}
	if expect.CanonicalHistoryFNV1a64 != "" {
		wire, err := sp.EncodeHistory(initial, certified, history)
		if err != nil {
			failures = append(failures, "wire encode: "+err.Error())
		} else {
			if digest := sp.FNV1a64(wire); digest != expect.CanonicalHistoryFNV1a64 {
				failures = append(failures, fmt.Sprintf("canonical_history_fnv1a64: expected %s, got %s", expect.CanonicalHistoryFNV1a64, digest))
			}
			decoded, err := sp.DecodeHistory(wire)
			if err != nil { failures = append(failures, "wire decode: "+err.Error())
			} else if !reflect.DeepEqual(decoded.History.Replay(decoded.InitialOptionSpace), state) {
				failures = append(failures, "wire_replay: decoded history produced a different state")
			}
			if _, err := sp.DecodeHistory(append(append([]byte(nil), wire...), 0)); err == nil {
				failures = append(failures, "wire_decode: accepted trailing bytes")
			}
		}
	}
	return failures
}

func runSubHistory(input *subHistory) (*sp.Arbiter, []string) {
	a := sp.NewArbiter(ids(input.InitialOptionSpace), rules(input.CertifiedRules))
	return a, execute(a, input.Events)
}

func runFixture(path string) ([]string, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return nil, err
	}
	var input fixture
	if err := json.Unmarshal(data, &input); err != nil {
		return nil, err
	}
	if input.HistoryA != nil || input.HistoryB != nil {
		if input.HistoryA == nil || input.HistoryB == nil {
			return []string{"meld fixture requires history_a and history_b"}, nil
		}
		a, af := runSubHistory(input.HistoryA)
		b, bf := runSubHistory(input.HistoryB)
		failures := append(af, bf...)
		melded := a.History().Meld(b.History())
		if input.ExpectMeldedHistoryLen != nil && melded.Len() != *input.ExpectMeldedHistoryLen {
			failures = append(failures, fmt.Sprintf("melded history len: expected %d, got %d", *input.ExpectMeldedHistoryLen, melded.Len()))
		}
		return failures, nil
	}
	if input.Manual {
		if input.Invariant == "" || input.Explanation == "" {
			return []string{"manual fixture lacks invariant or explanation"}, nil
		}
		return nil, nil
	}
	initial := ids(input.InitialOptionSpace)
	certified := rules(input.CertifiedRules)
	a := sp.NewArbiter(initial, certified)
	failures := execute(a, input.Events)
	failures = append(failures, checkExpected(a, initial, certified, input.Expect)...)
	return failures, nil
}

func defaultFixtureDir() string {
	return filepath.Join("..", "experiments", "flat", "fixtures")
}

func main() {
	dir := defaultFixtureDir()
	if len(os.Args) > 1 {
		dir = os.Args[1]
	}
	paths, err := filepath.Glob(filepath.Join(dir, "*.json"))
	if err != nil {
		panic(err)
	}
	if len(paths) == 0 {
		panic(errors.New("no fixture files found in " + dir))
	}
	sort.Strings(paths)
	passed, failed := 0, 0
	for _, path := range paths {
		failures, err := runFixture(path)
		name := strings.TrimSuffix(filepath.Base(path), filepath.Ext(path))
		if err != nil {
			failures = append(failures, err.Error())
		}
		if len(failures) == 0 {
			fmt.Println("PASS ", name)
			passed++
			continue
		}
		fmt.Println("FAIL ", name)
		for _, failure := range failures {
			fmt.Println("      -", failure)
		}
		failed++
	}
	fmt.Printf("\n%d passed, %d failed, %d total\n", passed, failed, passed+failed)
	if failed != 0 {
		os.Exit(1)
	}
}
