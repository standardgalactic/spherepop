/* sugar.h — Surface-calculus sugar: every function here expands to a
 * short, fixed sequence of primitive events and introduces no new event
 * kind. Mirrors spherepop-kernel/src/sugar.rs and run_python.py's
 * link/unlink/choice/merge/set_meta.
 */
#ifndef SP_FIXTURES_SUGAR_H
#define SP_FIXTURES_SUGAR_H

#include "kernel.h"

/* Each *_events() function fills `out` (caller-provided array, must have
 * capacity `cap`) and returns the number of events written. */
size_t sugar_link(ObjectId a, ObjectId b, const char *tag, Event *out, size_t cap);
size_t sugar_unlink(ObjectId a, ObjectId b, Event *out, size_t cap);
size_t sugar_choice(ObjectId taken, ObjectId rejected, Event *out, size_t cap);
size_t sugar_merge(ObjectId a, ObjectId b, const char *rule, Event *out, size_t cap);
size_t sugar_set_meta(ObjectId object, ObjectId key, Event *out, size_t cap);

#endif /* SP_FIXTURES_SUGAR_H */
