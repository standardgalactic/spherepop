package spherepop

type UnionFind struct{ parent map[ObjectID]ObjectID }

func NewUnionFind() *UnionFind {
	return &UnionFind{parent: make(map[ObjectID]ObjectID)}
}

func (u *UnionFind) root(x ObjectID) ObjectID {
	p, ok := u.parent[x]
	if !ok {
		u.parent[x] = x
		return x
	}
	if p != x {
		u.parent[x] = u.root(p)
	}
	return u.parent[x]
}

func (u *UnionFind) Union(a, b ObjectID) {
	ra, rb := u.root(a), u.root(b)
	if ra != rb {
		u.parent[ra] = rb
	}
}

func (u *UnionFind) SameClass(a, b ObjectID) bool {
	return u.root(a) == u.root(b)
}

func CollapseQuotient(h History) *UnionFind {
	uf := NewUnionFind()
	for _, e := range h.events {
		if e.Kind == Bind && e.A != nil && e.B != nil && (e.Tag == nil || *e.Tag != "__meta__") {
			uf.Union(*e.A, *e.B)
		}
	}
	return uf
}

func CollapseQuotientHonoringRefusals(h History) *UnionFind {
	type pair struct{ a, b ObjectID }
	withdrawn := make(map[pair]struct{})
	for _, e := range h.events {
		if e.Kind == Refuse && e.A != nil && e.B != nil && e.Reason != nil && *e.Reason == "relation withdrawn" {
			withdrawn[pair{*e.A, *e.B}] = struct{}{}
			withdrawn[pair{*e.B, *e.A}] = struct{}{}
		}
	}
	uf := NewUnionFind()
	for _, e := range h.events {
		if e.Kind != Bind || e.A == nil || e.B == nil || (e.Tag != nil && *e.Tag == "__meta__") {
			continue
		}
		if _, refused := withdrawn[pair{*e.A, *e.B}]; !refused {
			uf.Union(*e.A, *e.B)
		}
	}
	return uf
}

func CollapseMeta(h History) map[ObjectID][]ObjectID {
	out := make(map[ObjectID][]ObjectID)
	for _, e := range h.events {
		if e.Kind == Bind && e.A != nil && e.B != nil && e.Tag != nil && *e.Tag == "__meta__" {
			out[*e.A] = append(out[*e.A], *e.B)
		}
	}
	return out
}

// CollapseIdentity is the finest rule and returns a defensive history copy.
func CollapseIdentity(h History) []Event { return h.Events() }
