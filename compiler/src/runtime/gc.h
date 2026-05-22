/* gc.h — Mark-and-sweep GC for Value objects.
 *
 * Values use reference counting (val_retain/val_release) for normal
 * management. The GC handles cycles produced by closures and bubble
 * references. It is a stop-the-world mark-and-sweep over the value graph.
 */
#ifndef SP_GC_H
#define SP_GC_H
#include <stdbool.h>

struct Value;
struct EvalContext;

void gc_collect(struct EvalContext *ctx);
void gc_register(struct Value *v);
void gc_mark_root(struct Value *v);
int  gc_live_count(void);

#endif
