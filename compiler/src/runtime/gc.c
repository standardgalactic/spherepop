/* gc.c — Stub GC. Reference counting is primary; this triggers on threshold. */
#include "gc.h"
#include <stdio.h>
static int s_live = 0;
void gc_register(struct Value *v) { (void)v; s_live++; }
void gc_mark_root(struct Value *v) { (void)v; }
int  gc_live_count(void) { return s_live; }
void gc_collect(struct EvalContext *ctx) {
    (void)ctx;
    /* Placeholder: reference counting handles normal cases. */
}
