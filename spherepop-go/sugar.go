package spherepop

func Link(a, b ObjectID, relation string) Event {
	return BindEvent(a, b, relation)
}

func Unlink(a, b ObjectID) Event {
	return RefuseBindEvent(a, b, "relation withdrawn")
}

func Choice(taken, rejected ObjectID) []Event {
	return []Event{
		PopEvent(taken),
		RefuseEvent(rejected, "not selected by Choice"),
	}
}

func Merge(a, b ObjectID, id RuleID) []Event {
	return []Event{BindEvent(a, b, "merge"), CollapseEvent(id)}
}

func SetMeta(object, key ObjectID) Event {
	return BindEvent(object, key, "__meta__")
}
