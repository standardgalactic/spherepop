/* sugar.c — see sugar.h. */
#include "sugar.h"

size_t sugar_link(ObjectId a, ObjectId b, const char *tag, Event *out, size_t cap) {
    (void)cap;
    out[0] = event_bind(a, b, tag);
    return 1;
}

size_t sugar_unlink(ObjectId a, ObjectId b, Event *out, size_t cap) {
    (void)cap;
    /* Unlink(a,b) is Refuse(Bind(a,b)): it documents withdrawal, it does
     * not delete the original Bind fact from the history. */
    out[0] = event_refuse_bind(a, b, "relation withdrawn");
    return 1;
}

size_t sugar_choice(ObjectId taken, ObjectId rejected, Event *out, size_t cap) {
    (void)cap;
    out[0] = event_pop(taken);
    out[1] = event_refuse(rejected, "not selected by Choice");
    return 2;
}

size_t sugar_merge(ObjectId a, ObjectId b, const char *rule, Event *out, size_t cap) {
    (void)cap;
    /* Merge_c(a,b) := Collapse_c(Bind(a,b)) exactly. */
    out[0] = event_bind(a, b, "merge");
    out[1] = event_collapse(rule);
    return 2;
}

size_t sugar_set_meta(ObjectId object, ObjectId key, Event *out, size_t cap) {
    (void)cap;
    out[0] = event_bind(object, key, "__meta__");
    return 1;
}
