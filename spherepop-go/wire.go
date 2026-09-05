package spherepop

import (
	"bytes"
	"encoding/binary"
	"errors"
	"fmt"
	"sort"
	"unicode/utf8"
)

var wireMagic = [8]byte{'S', 'P', 'H', 'I', 'S', 'T', '1', 0}

type WireWorld struct {
	InitialOptionSpace []ObjectID
	CertifiedRules     []RuleID
	History            History
}

func putString(out *bytes.Buffer, value string) error {
	data := []byte(value)
	if uint64(len(data)) > uint64(^uint32(0)) {
		return errors.New("SPHIST/1 string too long")
	}
	_ = binary.Write(out, binary.BigEndian, uint32(len(data)))
	out.Write(data)
	return nil
}

func EncodeHistory(initial []ObjectID, rules []RuleID, history History) ([]byte, error) {
	omega := append([]ObjectID(nil), initial...)
	sort.Slice(omega, func(i, j int) bool { return omega[i] < omega[j] })
	for i := 1; i < len(omega); i++ {
		if omega[i] == omega[i-1] { return nil, errors.New("duplicate initial option") }
	}
	certified := append([]RuleID(nil), rules...)
	sort.Slice(certified, func(i, j int) bool { return certified[i] < certified[j] })
	for i := 1; i < len(certified); i++ {
		if certified[i] == certified[i-1] { return nil, errors.New("duplicate certified rule") }
	}
	if uint64(len(omega)) > uint64(^uint32(0)) || uint64(len(certified)) > uint64(^uint32(0)) || uint64(history.Len()) > uint64(^uint32(0)) {
		return nil, errors.New("SPHIST/1 collection too large")
	}
	out := bytes.NewBuffer(nil)
	out.Write(wireMagic[:])
	_ = binary.Write(out, binary.BigEndian, uint32(len(omega)))
	for _, id := range omega { _ = binary.Write(out, binary.BigEndian, uint64(id)) }
	_ = binary.Write(out, binary.BigEndian, uint32(len(certified)))
	for _, rule := range certified { if err := putString(out, string(rule)); err != nil { return nil, err } }
	_ = binary.Write(out, binary.BigEndian, uint32(history.Len()))
	for _, event := range history.events {
		out.WriteByte(byte(event.Kind))
		switch event.Kind {
		case Pop:
			_ = binary.Write(out, binary.BigEndian, uint64(*event.A))
		case Refuse:
			_ = binary.Write(out, binary.BigEndian, uint64(*event.A))
			if event.B == nil { out.WriteByte(0) } else { out.WriteByte(1); _ = binary.Write(out, binary.BigEndian, uint64(*event.B)) }
			if err := putString(out, *event.Reason); err != nil { return nil, err }
		case Bind:
			_ = binary.Write(out, binary.BigEndian, uint64(*event.A))
			_ = binary.Write(out, binary.BigEndian, uint64(*event.B))
			if err := putString(out, *event.Tag); err != nil { return nil, err }
		case Collapse:
			if err := putString(out, string(*event.Rule)); err != nil { return nil, err }
		default:
			return nil, fmt.Errorf("unknown event kind %d", event.Kind)
		}
	}
	return out.Bytes(), nil
}

type wireReader struct { data []byte; offset int }
func (r *wireReader) take(n int) ([]byte, error) { if n < 0 || r.offset+n > len(r.data) { return nil, errors.New("truncated SPHIST/1 envelope") }; b:=r.data[r.offset:r.offset+n]; r.offset+=n; return b,nil }
func (r *wireReader) u8() (byte,error) { b,e:=r.take(1); if e!=nil{return 0,e}; return b[0],nil }
func (r *wireReader) u32() (uint32,error) { b,e:=r.take(4); if e!=nil{return 0,e}; return binary.BigEndian.Uint32(b),nil }
func (r *wireReader) u64() (uint64,error) { b,e:=r.take(8); if e!=nil{return 0,e}; return binary.BigEndian.Uint64(b),nil }
func (r *wireReader) str() (string,error) { n,e:=r.u32(); if e!=nil{return "",e}; b,e:=r.take(int(n)); if e!=nil{return "",e}; if !utf8.Valid(b){return "",errors.New("invalid UTF-8 in SPHIST/1 envelope")}; return string(b),nil }

func DecodeHistory(data []byte) (WireWorld, error) {
	r := &wireReader{data:data}
	magic, err := r.take(len(wireMagic)); if err != nil || !bytes.Equal(magic, wireMagic[:]) { return WireWorld{}, errors.New("invalid SPHIST/1 magic") }
	nOmega,err:=r.u32(); if err!=nil{return WireWorld{},err}; omega:=make([]ObjectID,nOmega)
	for i:=range omega { v,e:=r.u64(); if e!=nil{return WireWorld{},e}; omega[i]=ObjectID(v); if i>0 && omega[i]<=omega[i-1]{return WireWorld{},errors.New("non-canonical option ordering")} }
	nRules,err:=r.u32(); if err!=nil{return WireWorld{},err}; rules:=make([]RuleID,nRules)
	for i:=range rules { v,e:=r.str(); if e!=nil{return WireWorld{},e}; rules[i]=RuleID(v); if i>0 && rules[i]<=rules[i-1]{return WireWorld{},errors.New("non-canonical rule ordering")} }
	nEvents,err:=r.u32(); if err!=nil{return WireWorld{},err}; history:=NewHistory()
	for i:=uint32(0); i<nEvents; i++ {
		kind,e:=r.u8(); if e!=nil{return WireWorld{},e}; var event Event
		switch EventKind(kind) {
		case Pop: a,e:=r.u64(); if e!=nil{return WireWorld{},e}; event=PopEvent(ObjectID(a))
		case Refuse:
			a,e:=r.u64(); if e!=nil{return WireWorld{},e}; flag,e:=r.u8(); if e!=nil{return WireWorld{},e}; if flag>1{return WireWorld{},errors.New("invalid Refuse has_b flag")}; var b uint64; if flag==1 { b,e=r.u64(); if e!=nil{return WireWorld{},e} }; reason,e:=r.str(); if e!=nil{return WireWorld{},e}; if flag==1 { event=RefuseBindEvent(ObjectID(a),ObjectID(b),reason) } else { event=RefuseEvent(ObjectID(a),reason) }
		case Bind: a,e:=r.u64(); if e!=nil{return WireWorld{},e}; b,e:=r.u64(); if e!=nil{return WireWorld{},e}; tag,e:=r.str(); if e!=nil{return WireWorld{},e}; event=BindEvent(ObjectID(a),ObjectID(b),tag)
		case Collapse: rule,e:=r.str(); if e!=nil{return WireWorld{},e}; event=CollapseEvent(RuleID(rule))
		default: return WireWorld{},fmt.Errorf("invalid event kind %d",kind)
		}
		event.Pos=LogPos(i); history.append(event)
	}
	if r.offset!=len(data){return WireWorld{},errors.New("trailing bytes in SPHIST/1 envelope")}
	return WireWorld{omega,rules,history},nil
}

func FNV1a64(data []byte) string {
	var digest uint64 = 0xcbf29ce484222325
	for _, b := range data { digest ^= uint64(b); digest *= 0x100000001b3 }
	return fmt.Sprintf("%016x",digest)
}
